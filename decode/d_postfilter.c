/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "defines.h"
#include "comp.h"
#include "phase.h"
#include "sine.h"
#include "postfilter.h"

#define BG_THRESH 40.0f
#define BG_BETA    0.1f
#define BG_MARGIN  6.0f

void postfilter(MODEL *model, float *bg_est) {
    int m;
    
    /* determine average energy across spectrum */

    float e = 1E-12f;

    for (m = 1; m <= model->L; m++) {
        e += model->A[m] * model->A[m];
    }
    
    e = 10.0f * log10f(e / model->L);

    /* If beneath threshold, update bg estimate.  The idea
       of the threshold is to prevent updating during high level
       speech. */

    if ((e < BG_THRESH) && !model->voiced)
        *bg_est = *bg_est * (1.0f - BG_BETA) + e * BG_BETA;

    /* now mess with phases during voiced frames to make any harmonics
       less then our background estimate unvoiced.
     */

    float thresh = powf(10.0f, (*bg_est + BG_MARGIN) / 20.0f);
    
    if (model->voiced) {
        for (m = 1; m <= model->L; m++) {
            if (model->A[m] < thresh) {
                model->phi[m] = TAU * (float) codec2_rand() / CODEC2_RAND_MAX;
            }
        }
    }
}
