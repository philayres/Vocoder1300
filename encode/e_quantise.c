/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "defines.h"
#include "quantise.h"
#include "kiss_fft.h"

#define LSP_DELTA 0.01f         /* grid spacing for LSP root searches */

int lsp_bits_encode(int i) {
    return lsp_cb[i].log2m;
}

static void autocorrelate(float Sn[], float Rn[], int Nsam, int order) {
    int i, j;

    for (j = 0; j < order + 1; j++) {
        Rn[j] = 0.0f;

        for (i = 0; i < Nsam - j; i++)
            Rn[j] += Sn[i] * Sn[i + j];
    }
}

static void levinson_durbin(float R[], float lpcs[], int order) {
    float a[order + 1][order + 1];
    float sum, e, k;
    int i, j;

    e = R[0];

    for (i = 1; i <= order; i++) {
        sum = 0.0f;

        for (j = 1; j <= i - 1; j++)
            sum += a[i - 1][j] * R[i - j];

        k = -1.0 * (R[i] + sum) / e;
        
        if (fabsf(k) > 1.0f)
            k = 0.0f;

        a[i][i] = k;

        for (j = 1; j <= i - 1; j++)
            a[i][j] = a[i - 1][j] + k * a[i - 1][i - j];

        e *= (1 - k * k);
    }

    for (i = 1; i <= order; i++)
        lpcs[i] = a[order][i];

    lpcs[0] = 1.0f;
}

static float cheb_poly_eva(float *coef, float x, int order) {
    int i;
    float *t, *u, *v, sum;
    float T[(order / 2) + 1];

    t = T;
    *t++ = 1.0f;
    u = t--;
    *u++ = x;
    v = u--;

    for (i = 2; i <= order / 2; i++)
        *v++ = (2.0f * x)*(*u++) - *t++;

    sum = 0.0f;
    t = T;

    for (i = 0; i <= order / 2; i++)
        sum += coef[(order / 2) - i]**t++;

    return sum;
}

int quantise(const float * cb, float vec[], float w[], int k, int m, float *se) {
    float e;
    float diff;
    int i, j;

    int besti = 0;
    float beste = 1E32f;

    for (j = 0; j < m; j++) {
        e = 0.0f;

        for (i = 0; i < k; i++) {
            diff = cb[j * k + i] - vec[i];
            e += powf(diff * w[i], 2.0f);
        }

        if (e < beste) {
            beste = e;
            besti = j;
        }
    }

    *se += beste;

    return besti;
}

int encode_Wo(float Wo, int bits) {
    int index, Wo_levels = 1 << bits;
    float Wo_min = (TAU / P_MAX);
    float Wo_max = (TAU / P_MIN);
    float norm;

    norm = (Wo - Wo_min) / (Wo_max - Wo_min);
    index = floorf(Wo_levels * norm + 0.5f);

    if (index < 0)
        index = 0;

    if (index > (Wo_levels - 1))
        index = Wo_levels - 1;

    return index;
}

float speech_to_uq_lsps(float lsp[], float ak[], float Sn[], float w[], int order) {
    int i, roots;
    float Wn[M];
    float R[order + 1];

    float e = 0.0f;

    for (i = 0; i < M; i++) {
        Wn[i] = Sn[i] * w[i];
        e += Wn[i] * Wn[i];
    }

    if (e == 0.0f) {
        for (i = 0; i < order; i++)
            lsp[i] = (M_PI / order) * (float) i;

        return 0.0f;
    }

    autocorrelate(Wn, R, M, order);
    levinson_durbin(R, ak, order);

    float E = 0.0f;

    for (i = 0; i <= order; i++)
        E += ak[i] * R[i];

    for (i = 0; i <= order; i++)
        ak[i] *= powf(0.994f, (float) i);

    roots = lpc_to_lsp(ak, order, lsp, 5, LSP_DELTA);

    if (roots != order) {
        for (i = 0; i < order; i++)
            lsp[i] = (M_PI / order) * (float) i;
    }

    return E;
}

int lpc_to_lsp(float *a, int order, float *freq, int nb, float delta) {
    float psuml, psumr, psumm, temp_xr, xl, xr, xm = 0;
    float temp_psumr;
    int i, j, m, flag, k;
    float *px;
    float *qx;
    float *p;
    float *q;
    float *pt;
    int roots = 0;
    float Q[order + 1];
    float P[order + 1];

    flag = 1;
    m = order / 2;
    
    /* Allocate memory space for polynomials */

    px = P;
    qx = Q;
    p = px;
    q = qx;
    *px++ = 1.0f;
    *qx++ = 1.0f;
    
    for (i = 1; i <= m; i++) {
        *px++ = a[i] + a[order + 1 - i]-*p++;
        *qx++ = a[i] - a[order + 1 - i]+*q++;
    }
    
    px = P;
    qx = Q;
    
    for (i = 0; i < m; i++) {
        *px = 2.0f * *px;
        *qx = 2.0f * *qx;
        px++;
        qx++;
    }
    
    px = P;
    qx = Q;

    xr = 0.0f;
    xl = 1.0f;


    for (j = 0; j < order; j++) {
        if (j % 2)
            pt = qx;
        else
            pt = px;

        psuml = cheb_poly_eva(pt, xl, order);
        flag = 1;

        while (flag && (xr >= -1.0f)) {
            xr = xl - delta;
            psumr = cheb_poly_eva(pt, xr, order);
            temp_psumr = psumr;
            temp_xr = xr;

            if (((psumr * psuml) < 0.0f) || (psumr == 0.0f)) {
                roots++;

                psumm = psuml;
                for (k = 0; k <= nb; k++) {
                    xm = (xl + xr) / 2.0f;
                    psumm = cheb_poly_eva(pt, xm, order);

                    if (psumm * psuml > 0.0f) {
                        psuml = psumm;
                        xl = xm;
                    } else {
                        psumr = psumm;
                        xr = xm;
                    }
                }

                freq[j] = xm;
                xl = xm;
                flag = 0;
            } else {
                psuml = temp_psumr;
                xl = temp_xr;
            }
        }
    }

    /* convert from x domain to radians */

    for (i = 0; i < order; i++) {
        freq[i] = acosf(freq[i]);
    }

    return roots;
}

int encode_energy(float e, int bits) {
    int index, e_levels = 1 << bits;
    float e_min = E_MIN_DB;
    float e_max = E_MAX_DB;
    float norm;

    e = 10.0f * log10f(e);
    norm = (e - e_min) / (e_max - e_min);
    index = floorf(e_levels * norm + 0.5f);

    if (index < 0)
        index = 0;
    
    if (index > (e_levels - 1))
        index = e_levels - 1;

    return index;
}

void encode_lsps_scalar(int indexes[], float lsp[], int order) {
    int i, k, m;
    float wt[1];
    float lsp_hz[order];
    const float *cb;
    float se;

    for (i = 0; i < order; i++)
        lsp_hz[i] = (4000.0f / M_PI) * lsp[i];

    wt[0] = 1.0f;
    
    for (i = 0; i < order; i++) {
        k = lsp_cb[i].k;
        m = lsp_cb[i].m;
        cb = lsp_cb[i].cb;
        indexes[i] = quantise(cb, &lsp_hz[i], wt, k, m, &se);
    }
}

