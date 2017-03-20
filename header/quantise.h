/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */
#ifndef __QUANTISE__
#define __QUANTISE__

#include "comp.h"
#include "kiss_fft.h"

#define WO_BITS     7
#define WO_LEVELS   (1<<WO_BITS)
#define E_BITS      5
#define E_LEVELS    (1<<E_BITS)
#define E_MIN_DB   -10.0f
#define E_MAX_DB    40.0f
#define LSP_SCALAR_INDEXES    10
#define LPCPF_GAMMA 0.5f
#define LPCPF_BETA  0.2f

int lpc_to_lsp(float *, int, float *, int, float);
void lsp_to_lpc(float *, float *, int);
void apply_lpc_correction(MODEL *);
int lsp_bits_encode(int);
int lsp_bits_decode(int);
void aks_to_M2(kiss_fft_cfg, float [], int, MODEL *, float, float *, int, int, int, float, float, COMP []);
int encode_Wo(float, int);
float decode_Wo(int, int);
void encode_lsps_scalar(int [], float [], int);
void decode_lsps_scalar(float [], int [], int);
int encode_energy(float, int);
float decode_energy(int, int);
void pack_natural_or_gray(unsigned char *, unsigned int *, int, unsigned int, unsigned int);
int unpack_natural_or_gray(const unsigned char *, unsigned int *, unsigned int, unsigned int);
float speech_to_uq_lsps(float [], float [], float [], float [], int);
int check_lsp_order(float [], int);
void bw_expand_lsps(float [], int, float, float);

#endif
