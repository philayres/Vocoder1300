/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */
#include <math.h>
#include <string.h>

#include "defines.h"
#include "interp.h"
#include "quantise.h"

void interp_Wo2(MODEL *interp, MODEL *prev, MODEL *next, float weight) {
    if (interp->voiced && !prev->voiced && !next->voiced) {
        interp->voiced = 0;
    }

    if (interp->voiced) {
        if (prev->voiced && next->voiced)
            interp->Wo = (1.0f - weight) * prev->Wo + weight * next->Wo;

        if (!prev->voiced && next->voiced)
            interp->Wo = next->Wo;

        if (prev->voiced && !next->voiced)
            interp->Wo = prev->Wo;
    } else {
        interp->Wo = TAU / P_MAX;
    }

    interp->L = M_PI / interp->Wo;
}

float interp_energy2(float prev_e, float next_e, float weight) {
    return powf(10.0f, (1.0f - weight) * log10f(prev_e) + weight * log10f(next_e));
}

void interpolate_lsp_ver2(float interp[], float prev[], float next[], float weight, int order) {
    int i;

    for (i = 0; i < order; i++)
        interp[i] = (1.0f - weight) * prev[i] + weight * next[i];
}
