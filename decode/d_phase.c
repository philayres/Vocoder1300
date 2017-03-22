/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */

#include "defines.h"
#include "comp.h"
#include "kiss_fft.h"
#include "phase.h"

static unsigned long next = 1;

int codec2_rand(void) {
    next = next * 1103515245 + 12345;
    return ((unsigned) (next / 65536) % 32768);
}

void phase_synth_zero_order(MODEL *model, float *ex_phase, COMP A[]) {
    COMP Ex[MAX_AMP + 1];
    COMP A_[MAX_AMP + 1];
    COMP H[MAX_AMP + 1];
    int m, b;

    float r = TAU / FFT_SIZE;

    /* Sample phase at harmonics */

    for (m = 1; m <= model->L; m++) {
        b = (int) (m * model->Wo / r + 0.5f);
        H[m].real = A[b].real;;
        H[m].imag = -A[b].imag;
    }

    ex_phase[0] += (model->Wo * N);
    ex_phase[0] -= (TAU * floorf(ex_phase[0] / TAU + 0.5f));

    for (m = 1; m <= model->L; m++) {

        /* generate excitation */

        if (model->voiced) {
            Ex[m].real = cosf(ex_phase[0] * m);
            Ex[m].imag = sinf(ex_phase[0] * m);
        } else {
            /* When a few samples were tested I found that LPC filter
               phase is not needed in the unvoiced case, but no harm in
               keeping it.
             */
            float phi = TAU * (float) codec2_rand() / CODEC2_RAND_MAX;
            Ex[m].real = cosf(phi);
            Ex[m].imag = sinf(phi);
        }

        /* filter using LPC filter */

        A_[m].real = H[m].real * Ex[m].real - H[m].imag * Ex[m].imag;
        A_[m].imag = H[m].imag * Ex[m].real + H[m].real * Ex[m].imag;

        /* modify sinusoidal phase */

        model->phi[m] = atan2f(A_[m].imag, A_[m].real + 1E-12f);
    }
}

