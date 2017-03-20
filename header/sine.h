/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */
#ifndef __SINE__
#define __SINE__

#include "defines.h"
#include "comp.h"
#include "kiss_fft.h"

void dft_speech(kiss_fft_cfg, COMP [], float [], float []);
void two_stage_pitch_refinement(MODEL *, COMP []);
void estimate_amplitudes(MODEL *, COMP [], COMP [], int);
float est_voicing_mbe(MODEL *, COMP Sw[], COMP [], COMP [], COMP []);
void synthesise(kiss_fft_cfg, float [], MODEL *, float [], int);

#endif
