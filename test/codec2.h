/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */

#ifndef __CODEC2__
#define  __CODEC2__

#ifdef __cplusplus
extern "C"
{
#endif

int codec2_create(void);
void codec2_destroy(void);
void codec2_encode(unsigned char *, short [], unsigned int charbits);
void codec2_decode(short [], const unsigned char *);
void codec2_decode_ber(short [], const unsigned char *, float);
int codec2_samples_per_frame(void);
int codec2_bits_per_frame(void);
void codec2_set_lpc_post_filter(int, int, float, float);
int codec2_get_spare_bit_index(void);
int codec2_rebuild_spare_bit(int []);
void codec2_set_natural_or_gray(int);
float codec2_get_energy(const unsigned char *);
float codec2_get_sum_beste(void);

#ifdef __cplusplus
}
#endif
#endif
