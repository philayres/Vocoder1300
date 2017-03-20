/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __CODEC2__
#define  __CODEC2__

int codec2_create(void);
void codec2_destroy(void);
void codec2_encode(unsigned char *, short []);
void codec2_decode(short [], const unsigned char *);
void codec2_decode_ber(short [], const unsigned char *, float);
int codec2_samples_per_frame(void);
int codec2_bits_per_frame(void);
void codec2_set_lpc_post_filter(int, int, float, float);
int codec2_get_spare_bit_index(void);
int codec2_rebuild_spare_bit(int []);
void codec2_set_natural_or_gray(int);
void codec2_set_softdec(float *);
float codec2_get_energy(const unsigned char *);

#endif

#ifdef __cplusplus
}
#endif
