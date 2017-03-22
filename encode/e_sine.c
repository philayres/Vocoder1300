/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */

#include "defines.h"
#include "sine.h"
#include "kiss_fft.h"

/* BSS Storage */

static COMP bss_Sw_[FFT_SIZE];
static COMP bss_Ew[FFT_SIZE];
    
static void hs_pitch_refinement(MODEL *, COMP [], float, float, float);

void two_stage_pitch_refinement(MODEL *model, COMP Sw[]) {
    float pmin, pmax, pstep;

    /* Coarse refinement */

    pmax = TAU / model->Wo + 5.0f;
    pmin = TAU / model->Wo - 5.0f;
    pstep = 1.0f;
    hs_pitch_refinement(model, Sw, pmin, pmax, pstep);

    /* Fine refinement */

    pmax = TAU / model->Wo + 1.0f;
    pmin = TAU / model->Wo - 1.0f;
    pstep = 0.25f;
    hs_pitch_refinement(model, Sw, pmin, pmax, pstep);

    /* Limit range */

    if (model->Wo < (TAU / P_MAX)) {
        model->Wo = (TAU / P_MAX);
    }
    
    if (model->Wo > (TAU / P_MIN)) {
        model->Wo = (TAU / P_MIN);
    }

    model->L = floorf(M_PI / model->Wo);
}

static void hs_pitch_refinement(MODEL *model, COMP Sw[], float pmin, float pmax, float pstep) {
    float E, Wo, p;
    int m, b;

    float Wom = model->Wo;
    float Em = 0.0f;
    float one_on_r = 1.0f / (TAU / FFT_SIZE);

    for (p = pmin; p <= pmax; p += pstep) {
        E = 0.0f;
        Wo = TAU / p;

        /* Sum harmonic magnitudes */
        for (m = 1; m <= model->L; m++) {
            b = (int) (m * Wo * one_on_r + 0.5f);
            E += Sw[b].real * Sw[b].real + Sw[b].imag * Sw[b].imag;
        }
        
        /* Compare to see if this is a maximum */

        if (E > Em) {
            Em = E;
            Wom = Wo;
        }
    }

    model->Wo = Wom;
}

void dft_speech(kiss_fft_cfg fft_fwd_cfg, COMP Sw[], float Sn[], float w[]) {
    COMP sw[FFT_SIZE];
    int i;

    for (i = 0; i < FFT_SIZE; i++) {
        sw[i].real = 0.0f;
        sw[i].imag = 0.0f;
    }

    for (i = 0; i < (NW / 2); i++) {
        sw[i].real = Sn[i + M / 2] * w[i + M / 2];
    }
    
    for (i = 0; i < (NW / 2); i++) {
        sw[FFT_SIZE - NW / 2 + i].real = Sn[i + M / 2 - NW / 2] * w[i + M / 2 - NW / 2];
    }

    codec2_fft(fft_fwd_cfg, sw, Sw);
}

void estimate_amplitudes(MODEL *model, COMP Sw[], COMP W[], int est_phase) {
    COMP Am;
    float den;
    int i, m, am, bm, b, offset;

    float r = TAU / FFT_SIZE;
    float one_on_r = 1.0f / r;

    for (m = 1; m <= model->L; m++) {
        den = 0.0f;
        am = (int) ((m - 0.5f) * model->Wo * one_on_r + 0.5f);
        bm = (int) ((m + 0.5f) * model->Wo * one_on_r + 0.5f);
        b = (int) (m * model->Wo / r + 0.5f);

        den = 0.0f;
        Am.real = 0.0f;
        Am.imag = 0.0f;
        
        offset = FFT_SIZE / 2 - (int) (m * model->Wo * one_on_r + 0.5f);
        
        for (i = am; i < bm; i++) {
            den += Sw[i].real * Sw[i].real + Sw[i].imag * Sw[i].imag;
            Am.real += Sw[i].real * W[i + offset].real;
            Am.imag += Sw[i].imag * W[i + offset].real;
        }

        model->A[m] = sqrtf(den);

        /*
         * Not used if firmware (too expensive)
         */
        if (est_phase) {
            model->phi[m] = atan2f(Sw[b].imag, Sw[b].real);
        }
    }
}

float est_voicing_mbe(MODEL *model, COMP Sw[], COMP W[]) {
    COMP Am;
    float den;
    int i, l, al, bl, m, offset;

    float sig = 1E-4f;
    float sixty = 60.0f * (TAU / FS);   

    for (l = 1; l <= (model->L / 4); l++) {
        sig += model->A[l] * model->A[l];
    }
    
    for (i = 0; i < FFT_SIZE; i++) {
        bss_Sw_[i].real = 0.0f;
        bss_Sw_[i].imag = 0.0f;
        bss_Ew[i].real = 0.0f;
        bss_Ew[i].imag = 0.0f;
    }

    float Wo = model->Wo;
    float error = 1E-4f;

    /* Just test across the harmonics in the first 1000 Hz (L/4) */

    for (l = 1; l <= (model->L / 4); l++) {
        Am.real = 0.0f;
        Am.imag = 0.0f;
        den = 0.0f;
        al = ceilf((l - 0.5f) * Wo * FFT_SIZE / TAU);
        bl = ceilf((l + 0.5f) * Wo * FFT_SIZE / TAU);

        offset = (FFT_SIZE / 2) - l * Wo * FFT_SIZE / TAU + 0.5f;
        
        for (m = al; m < bl; m++) {
            Am.real += Sw[m].real * W[offset + m].real;
            Am.imag += Sw[m].imag * W[offset + m].real;
            den += W[offset + m].real * W[offset + m].real;
        }

        Am.real = Am.real / den;
        Am.imag = Am.imag / den;

        offset = (FFT_SIZE / 2) - l * Wo * FFT_SIZE / TAU + 0.5f;
        
        for (m = al; m < bl; m++) {
            bss_Sw_[m].real = Am.real * W[offset + m].real;
            bss_Sw_[m].imag = Am.imag * W[offset + m].real;
            
            bss_Ew[m].real = Sw[m].real - bss_Sw_[m].real;
            bss_Ew[m].imag = Sw[m].imag - bss_Sw_[m].imag;
            
            error += bss_Ew[m].real * bss_Ew[m].real;
            error += bss_Ew[m].imag * bss_Ew[m].imag;
        }
    }

    float snr = 10.0f * log10f(sig / error);
    
    if (snr > V_THRESH) {
        model->voiced = 1;
    } else {
        model->voiced = 0;
    }

    float elow = 1E-4f;
    float ehigh = 1E-4f;
    
    for (l = 1; l <= (model->L / 2); l++) {
        elow += model->A[l] * model->A[l];
    }
    
    for (l = (model->L / 2); l <= model->L; l++) {
        ehigh += model->A[l] * model->A[l];
    }
    
    float eratio = 10.0f * log10f(elow / ehigh);

    if (model->voiced == 0) {
        if (eratio > 10.0f) {
            model->voiced = 1;
        }
    } else if (model->voiced == 1) {
        if (eratio < -10.0f) {
            model->voiced = 0;
        } else if ((eratio < -4.0f) && (model->Wo <= sixty)) {
            model->voiced = 0;
        }
    }

    return snr;
}
