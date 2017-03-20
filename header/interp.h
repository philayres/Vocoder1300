/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */
#ifndef __INTERP__
#define __INTERP__

#include "kiss_fft.h"

void interp_Wo2(MODEL *, MODEL *, MODEL *, float);
float interp_energy2(float, float, float);
void interpolate_lsp_ver2(float [], float [], float [], float, int);

#endif
