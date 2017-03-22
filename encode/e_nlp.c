/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */

#include "defines.h"
#include "nlp.h"
#include "kiss_fft.h"
#include "codec2_fft.h"

/* BSS Storage */

static int bss_m;
static float bss_w[PMAX_M / DEC];       /* DFT window                   */
static float bss_sq[PMAX_M];            /* squared speech samples       */
static float bss_mem_x;
static float bss_mem_y;                 /* memory for notch filter      */
static float bss_mem_fir[NLP_NTAP];     /* decimation FIR filter memory */
static codec2_fft_cfg bss_fft_cfg;        /* kiss FFT config              */

/* 48 tap 600Hz low pass FIR filter coefficients */

static const float nlp_fir[] = {
    -1.0818124e-03f,
    -1.1008344e-03f,
    -9.2768838e-04f,
    -4.2289438e-04f,
    5.5034190e-04f,
    2.0029849e-03f,
    3.7058509e-03f,
    5.1449415e-03f,
    5.5924666e-03f,
    4.3036754e-03f,
    8.0284511e-04f,
    -4.8204610e-03f,
    -1.1705810e-02f,
    -1.8199275e-02f,
    -2.2065282e-02f,
    -2.0920610e-02f,
    -1.2808831e-02f,
    3.2204775e-03f,
    2.6683811e-02f,
    5.5520624e-02f,
    8.6305944e-02f,
    1.1480192e-01f,
    1.3674206e-01f,
    1.4867556e-01f,
    1.4867556e-01f,
    1.3674206e-01f,
    1.1480192e-01f,
    8.6305944e-02f,
    5.5520624e-02f,
    2.6683811e-02f,
    3.2204775e-03f,
    -1.2808831e-02f,
    -2.0920610e-02f,
    -2.2065282e-02f,
    -1.8199275e-02f,
    -1.1705810e-02f,
    -4.8204610e-03f,
    8.0284511e-04f,
    4.3036754e-03f,
    5.5924666e-03f,
    5.1449415e-03f,
    3.7058509e-03f,
    2.0029849e-03f,
    5.5034190e-04f,
    -4.2289438e-04f,
    -9.2768838e-04f,
    -1.1008344e-03f,
    -1.0818124e-03f
};

static float post_process_sub_multiples(COMP [], float, int, float *);

int nlp_create(int m) {
    int i;

    bss_m = m;  /* 320 samples */

    for (i = 0; i < m / DEC; i++) {
        bss_w[i] = 0.5f - 0.5f * cosf(2 * M_PI * i / (m / DEC - 1));
    }

    bss_mem_x = 0.0f;
    bss_mem_y = 0.0f;

    bss_fft_cfg = codec2_fft_alloc(FFT_SIZE, 0, NULL, NULL);

    if (bss_fft_cfg == NULL) {
        return 0;
    }

    return 1;
}

void nlp_destroy(void) {
    codec2_fft_free(bss_fft_cfg);
}

static float post_process_sub_multiples(COMP Fw[], float gmax, int gmax_bin, float *prev_Wo) {
    float thresh, lmax;
    int b, bmin, bmax, lmax_bin;

    int mult = 2;
    int min_bin = FFT_SIZE * DEC / P_MAX;
    int cmax_bin = gmax_bin;
    int prev_f0_bin = (int) (*prev_Wo * (4000.0f / M_PI) * (float)(FFT_SIZE * DEC) / (float)SAMPLE_RATE);

    while (gmax_bin / mult >= min_bin) {
        b = gmax_bin / mult; /* determine search interval */
        bmin = 0.8f * b;
        bmax = 1.2f * b;
        
        if (bmin < min_bin) {
            bmin = min_bin;
        }

        /* lower threshold to favor previous frames pitch estimate,
            this is a form of pitch tracking */

        if ((prev_f0_bin > bmin) && (prev_f0_bin < bmax)) {
            thresh = CNLP * 0.5f * gmax;
        } else {
            thresh = CNLP * gmax;
        }

        lmax = 0;
        lmax_bin = bmin;

        for (b = bmin; b <= bmax; b++) { /* look for maximum in interval */
            if (Fw[b].real > lmax) {
                lmax = Fw[b].real;
                lmax_bin = b;
            }
        }

        if (lmax > thresh) {
            if ((lmax > Fw[lmax_bin - 1].real) && (lmax > Fw[lmax_bin + 1].real)) {
                cmax_bin = lmax_bin;
            }
        }

        mult++;
    }

    return (float) cmax_bin * (float) SAMPLE_RATE / (float) (FFT_SIZE * DEC);
}

float nlp(
        float Sn[], /* input speech vector */
        float *pitch, /* estimated pitch period in samples */
        float *prev_Wo
        ) {
    float notch; /* current notch filter output    */
    COMP fw[FFT_SIZE]; /* DFT of squared signal (input)  */
    COMP Fw[FFT_SIZE]; /* DFT of squared signal (output) */
    float gmax;
    int i, j, gmax_bin;

    /* Square, notch filter at DC, and LP filter vector */

    for (i = bss_m - N; i < bss_m; i++) { /* square latest speech samples (240 to 320) */
        bss_sq[i] = (Sn[i] * Sn[i]);
    }

    /* notch filter at DC */
    
    for (i = bss_m - N; i < bss_m; i++) { 
        notch = bss_sq[i] - bss_mem_x;
        notch += COEFF * bss_mem_y;
        bss_mem_x = bss_sq[i];
        bss_mem_y = notch;
        bss_sq[i] = notch + 1.0f;
    }

    for (i = bss_m - N; i < bss_m; i++) { /* FIR filter vector */
        for (j = 0; j < NLP_NTAP - 1; j++) {
            bss_mem_fir[j] = bss_mem_fir[j + 1];
        }

        bss_mem_fir[NLP_NTAP - 1] = bss_sq[i];

        bss_sq[i] = 0.0f;

        for (j = 0; j < NLP_NTAP; j++) {
            bss_sq[i] += bss_mem_fir[j] * nlp_fir[j];
        }
    }

    for (i = 0; i < FFT_SIZE; i++) {
        fw[i].real = 0.0f;
        fw[i].imag = 0.0f;
    }
    
    /* Decimate by 5 and DFT */
    
    for (i = 0; i < bss_m / DEC; i++) {         /* 320 / 5 = 64 */
        fw[i].real = bss_sq[i * DEC] * bss_w[i];
    }

    codec2_fft(bss_fft_cfg, fw, Fw);

    for (i = 0; i < FFT_SIZE; i++) {
        Fw[i].real = Fw[i].real * Fw[i].real + Fw[i].imag * Fw[i].imag;
    }

    /* find global peak */

    gmax = 0.0f;
    gmax_bin = FFT_SIZE * DEC / P_MIN;

    for (i = FFT_SIZE * DEC / P_MAX; i <= (FFT_SIZE * DEC / P_MIN); i++) {
        if (Fw[i].real > gmax) {
            gmax = Fw[i].real;
            gmax_bin = i;
        }
    }

    float best_f0 = post_process_sub_multiples(Fw, gmax, gmax_bin, prev_Wo);

    /* Shift samples in buffer to make room for new samples */

    for (i = 0; i < bss_m - N; i++) {
        bss_sq[i] = bss_sq[i + N];
    }

    /* return pitch and F0 estimate */

    *pitch = (float) SAMPLE_RATE / best_f0;

    return best_f0;
}
