/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "defines.h"
#include "sine.h"
#include "nlp.h"
#include "quantise.h"
#include "phase.h"
#include "interp.h"
#include "postfilter.h"
#include "codec2.h"

static void analyse_one_frame(MODEL *, short []);
static void synthesise_one_frame(short [], MODEL *, COMP []);
static void ear_protection(float in_out[], int n);
static void make_analysis_window(kiss_fft_cfg, float [], COMP []);
static void make_synthesis_window(float []);
static void codec2_encode_1300(unsigned char *, short []);
static void codec2_decode_1300(short [], const unsigned char *, float);

/* BSS Storage */

MODEL bss_prev_model_dec; /* previous frame's model parameters         */
kiss_fft_cfg bss_fft_fwd_cfg; /* forward FFT config                        */
kiss_fft_cfg bss_fft_inv_cfg; /* inverse FFT config                        */
COMP bss_W[FFT_SIZE]; /* DFT of w[]                                */
float bss_w[M]; /* time domain hamming window                */
float bss_Pn[2 * N]; /* trapezoidal synthesis window              */
float bss_Sn[M]; /* input speech                              */
float bss_Sn_[2 * N]; /* synthesised output speech                 */
float bss_ex_phase; /* excitation model phase track              */
float bss_bg_est; /* background noise estimate for post filter */
float bss_prev_Wo_enc; /* previous frame's pitch estimate           */
float bss_prev_lsps_dec[LPC_ORD]; /* previous frame's LSPs                     */
float bss_prev_e_dec; /* previous frame's LPC energy               */
float bss_beta; /* LPC post filter parameters                */
float bss_gamma;
float bss_se;
int bss_gray; /* non-zero for gray encoding                */
int bss_lpc_pf; /* LPC post filter on                        */
int bss_bass_boost; /* LPC post filter bass boost                */

int codec2_create() {
    int i, l;

    for (i = 0; i < M; i++) {
        bss_Sn[i] = 1.0f;
    }
    
    for (i = 0; i < (N * 2); i++) {
        bss_Sn_[i] = 0.0f;
    }

    bss_fft_fwd_cfg = kiss_fft_alloc(FFT_SIZE, 0, NULL, NULL);

    make_analysis_window(bss_fft_fwd_cfg, bss_w, bss_W);
    make_synthesis_window(bss_Pn);

    bss_fft_inv_cfg = kiss_fft_alloc(FFT_SIZE, 1, NULL, NULL);

    bss_prev_Wo_enc = 0.0f;
    bss_bg_est = 0.0f;
    bss_ex_phase = 0.0f;
    bss_se = 0.0f;

    for (l = 1; l <= MAX_AMP; l++) {
        bss_prev_model_dec.A[l] = 0.0f;
    }

    bss_prev_model_dec.Wo = TAU / P_MAX;
    bss_prev_model_dec.L = M_PI / bss_prev_model_dec.Wo;
    bss_prev_model_dec.voiced = 0;

    for (i = 0; i < LPC_ORD; i++) {
        bss_prev_lsps_dec[i] = i * M_PI / (LPC_ORD + 1);
    }

    bss_prev_e_dec = 1.0f;

    if (nlp_create(M) == 0) {
        return 0;
    }

    bss_gray = 1;
    bss_lpc_pf = 1;
    bss_bass_boost = 1;

    bss_beta = LPCPF_BETA;
    bss_gamma = LPCPF_GAMMA;

    return 1;
}

void codec2_destroy() {
    nlp_destroy();
    free(bss_fft_fwd_cfg);
    free(bss_fft_inv_cfg);
}

int codec2_bits_per_frame() {
    return 52;
}

int codec2_samples_per_frame() {
    return 320;
}

void codec2_encode(unsigned char *bits, short speech[]) {
    codec2_encode_1300(bits, speech);
}

void codec2_decode(short speech[], const unsigned char *bits) {
    codec2_decode_ber(speech, bits, 0.0f);
}

void codec2_decode_ber(short speech[], const unsigned char *bits, float ber_est) {
    codec2_decode_1300(speech, bits, ber_est);
}

float codec2_get_energy(const unsigned char *bits) {
    unsigned int nbit = 1 + 1 + 1 + 1 + WO_BITS;
    int e_index = unpack_natural_or_gray(bits, &nbit, E_BITS, bss_gray);

    return decode_energy(e_index, E_BITS);
}

float codec2_get_sum_beste() {
    return bss_se;
}

void codec2_set_lpc_post_filter(int enable, int bass_boost, float beta, float gamma) {
    bss_lpc_pf = enable;
    bass_boost = bass_boost;
    bss_beta = beta;
    bss_gamma = gamma;
}

/*
 * Allows optional stealing of one of the voicing bits
 * for use as a spare bit
 */

int codec2_get_spare_bit_index() {
    return 2;
}

int codec2_rebuild_spare_bit(int unpacked_bits[]) {
    int v1 = unpacked_bits[1];
    int v3 = unpacked_bits[1 + 1 + 1];

    /* if either adjacent frame is voiced, make this one voiced */

    unpacked_bits[2] = (v1 || v3);

    return 0;
}

void codec2_set_natural_or_gray(int gray) {
    bss_gray = gray;
}

static void codec2_encode_1300(unsigned char * bits, short speech[]) {
    MODEL model;
    float lsps[LPC_ORD];
    float ak[LPC_ORD + 1];
    int lsp_indexes[LPC_ORD];
    int i;
    unsigned int nbit = 0;

    memset(bits, '\0', ((codec2_bits_per_frame() + 7) / 8));

    analyse_one_frame(&model, speech);
    pack_natural_or_gray(bits, &nbit, model.voiced, 1, bss_gray);

    analyse_one_frame(&model, &speech[N]);
    pack_natural_or_gray(bits, &nbit, model.voiced, 1, bss_gray);

    analyse_one_frame(&model, &speech[2 * N]);
    pack_natural_or_gray(bits, &nbit, model.voiced, 1, bss_gray);

    analyse_one_frame(&model, &speech[3 * N]);
    pack_natural_or_gray(bits, &nbit, model.voiced, 1, bss_gray);

    int Wo_index = encode_Wo(model.Wo, WO_BITS);
    pack_natural_or_gray(bits, &nbit, Wo_index, WO_BITS, bss_gray);

    float e = speech_to_uq_lsps(lsps, ak, bss_Sn, bss_w, LPC_ORD);
    int e_index = encode_energy(e, E_BITS);
    pack_natural_or_gray(bits, &nbit, e_index, E_BITS, bss_gray);

    bss_se = encode_lsps_scalar(lsp_indexes, lsps, LPC_ORD);

    for (i = 0; i < LSP_SCALAR_INDEXES; i++) {
        pack_natural_or_gray(bits, &nbit, lsp_indexes[i], lsp_bits_encode(i), bss_gray);
    }
}

static void codec2_decode_1300(short speech[], const unsigned char * bits, float ber_est) {
    MODEL model[4];
    COMP Aw[FFT_SIZE];
    float lsps[4][LPC_ORD];
    float ak[4][LPC_ORD + 1];
    int lsp_indexes[LPC_ORD];
    float e[4];
    float snr, weight;
    int i, j;
    unsigned int nbit = 0;

    for (i = 0; i < 4; i++) {
        for (j = 1; j <= MAX_AMP; j++) {
            model[i].A[j] = 0.0f;
        }
    }

    model[0].voiced = unpack_natural_or_gray(bits, &nbit, 1, bss_gray);
    model[1].voiced = unpack_natural_or_gray(bits, &nbit, 1, bss_gray);
    model[2].voiced = unpack_natural_or_gray(bits, &nbit, 1, bss_gray);
    model[3].voiced = unpack_natural_or_gray(bits, &nbit, 1, bss_gray);

    int Wo_index = unpack_natural_or_gray(bits, &nbit, WO_BITS, bss_gray);
    model[3].Wo = decode_Wo(Wo_index, WO_BITS);
    model[3].L = M_PI / model[3].Wo;

    int e_index = unpack_natural_or_gray(bits, &nbit, E_BITS, bss_gray);
    e[3] = decode_energy(e_index, E_BITS);

    for (i = 0; i < LSP_SCALAR_INDEXES; i++) {
        lsp_indexes[i] = unpack_natural_or_gray(bits, &nbit, lsp_bits_decode(i), bss_gray);
    }

    decode_lsps_scalar(&lsps[3][0], lsp_indexes, LPC_ORD);
    check_lsp_order(&lsps[3][0], LPC_ORD);
    bw_expand_lsps(&lsps[3][0], LPC_ORD, 50.0f, 100.0f);

    if (ber_est > 0.15f) {
        model[0].voiced = model[1].voiced = model[2].voiced = model[3].voiced = 0;
        e[3] = decode_energy(10, E_BITS);
        bw_expand_lsps(&lsps[3][0], LPC_ORD, 200.0f, 200.0f);
    }

    for (i = 0, weight = 0.25f; i < 3; i++, weight += 0.25f) {
        interpolate_lsp_ver2(&lsps[i][0], bss_prev_lsps_dec, &lsps[3][0], weight, LPC_ORD);
        interp_Wo2(&model[i], &bss_prev_model_dec, &model[3], weight);
        e[i] = interp_energy2(bss_prev_e_dec, e[3], weight);
    }

    for (i = 0; i < 4; i++) {
        lsp_to_lpc(&lsps[i][0], &ak[i][0], LPC_ORD);
        aks_to_M2(bss_fft_fwd_cfg, &ak[i][0], LPC_ORD, &model[i], e[i], &snr, 0,
                bss_lpc_pf, bss_bass_boost, bss_beta, bss_gamma, Aw);
        apply_lpc_correction(&model[i]);
        synthesise_one_frame(&speech[N * i], &model[i], Aw);
    }

    bss_prev_model_dec = model[3];
    bss_prev_e_dec = e[3];

    for (i = 0; i < LPC_ORD; i++) {
        bss_prev_lsps_dec[i] = lsps[3][i];
    }
}

static void synthesise_one_frame(short speech[], MODEL *model, COMP Aw[]) {
    int i;

    phase_synth_zero_order(model, &bss_ex_phase, Aw);
    postfilter(model, &bss_bg_est);
    synthesise(bss_fft_inv_cfg, bss_Sn_, model, bss_Pn, 1);
    ear_protection(bss_Sn_, N);

    for (i = 0; i < N; i++) {
        if (bss_Sn_[i] > 32767.0f) {
            speech[i] = 32767;
        } else if (bss_Sn_[i] < -32767.0f) {
            speech[i] = -32767;
        } else {
            speech[i] = bss_Sn_[i];
        }
    }
}

static void analyse_one_frame(MODEL *model, short speech[]) {
    COMP Sw[FFT_SIZE];
    float pitch;
    int i;

    /* Shift the old amplitude samples left 80 samples */

    for (i = 0; i < M - N; i++) {
        bss_Sn[i] = bss_Sn[i + N];
    }
    
    /* Add the new 80 amplitude samples to the end */

    for (i = 0; i < N; i++) {
        bss_Sn[i + M - N] = (float) speech[i];
    }
    
    /*
     * Hamming filter (Sn) * (w) and convert to Frequency Domain (Sw)
     */
    dft_speech(bss_fft_fwd_cfg, Sw, bss_Sn, bss_w);

    /*
     * Estimate pitch (Sn)
     */

    nlp(bss_Sn, &pitch, &bss_prev_Wo_enc);

    model->Wo = TAU / pitch;
    model->L = M_PI / model->Wo;

    /* estimate/refine model parameters and voicing */

    two_stage_pitch_refinement(model, Sw);
    estimate_amplitudes(model, Sw, bss_W, 0);
    est_voicing_mbe(model, Sw, bss_W);
    
    bss_prev_Wo_enc = model->Wo;
}

static void ear_protection(float in_out[], int n) {
    float gain;
    int i;

    float max_sample = 0.0f;

    for (i = 0; i < n; i++) {
        if (in_out[i] > max_sample) {
            max_sample = in_out[i];
        }
    }

    float over = max_sample / 30000.0f;

    if (over > 1.0f) {
        gain = 1.0f / (over * over);

        for (i = 0; i < n; i++) {
            in_out[i] *= gain;
        }
    }
}

static void make_analysis_window(kiss_fft_cfg fft_fwd_cfg, float w[], COMP W[]) {
    COMP wshift[FFT_SIZE];
    COMP temp;
    int i, j;

    float m = 0.0f;

    for (i = 0; i < (M / 2 - NW / 2); i++) {
        w[i] = 0.0f;
    }

    for (i = (M / 2 - NW / 2), j = 0; i < (M / 2 + NW / 2); i++, j++) {
        w[i] = 0.5f - 0.5f * cosf(TAU * j / (NW - 1));
        m += w[i] * w[i];
    }

    for (i = (M / 2 + NW / 2); i < M; i++) {
        w[i] = 0.0f;
    }

    m = 1.0f / sqrtf(m * FFT_SIZE);

    for (i = 0; i < M; i++) {
        w[i] *= m;
    }

    for (i = 0; i < FFT_SIZE; i++) {
        wshift[i].real = 0.0f;
        wshift[i].imag = 0.0f;
    }

    for (i = 0; i < (NW / 2); i++) {
        wshift[i].real = w[i + M / 2];
    }

    for (i = (FFT_SIZE - NW / 2), j = (M / 2 - NW / 2); i < FFT_SIZE; i++, j++) {
        wshift[i].real = w[j];
    }

    kiss_fft(fft_fwd_cfg, (COMP *) wshift, (COMP *) W);

    for (i = 0; i < (FFT_SIZE / 2); i++) {
        temp.real = W[i].real;
        temp.imag = W[i].imag;

        W[i].real = W[i + FFT_SIZE / 2].real;
        W[i].imag = W[i + FFT_SIZE / 2].imag;

        W[i + FFT_SIZE / 2].real = temp.real;
        W[i + FFT_SIZE / 2].imag = temp.imag;
    }
}

static void make_synthesis_window(float Pn[]) {
    int i;

    float win = 0.0f;
    
    for (i = 0; i < N / 2 - TW; i++) {
        Pn[i] = 0.0f;
    }
    
    win = 0.0f;
    
    for (i = N / 2 - TW; i < N / 2 + TW; win += 1.0 / (2 * TW), i++) {
        Pn[i] = win;
    }

    for (i = N / 2 + TW; i < 3 * N / 2 - TW; i++) {
        Pn[i] = 1.0f;
    }

    win = 1.0f;

    for (i = 3 * N / 2 - TW; i < 3 * N / 2 + TW; win -= 1.0f / (2 * TW), i++) {
        Pn[i] = win;
    }

    for (i = 3 * N / 2 + TW; i < 2 * N; i++) {
        Pn[i] = 0.0f;
    }
}
