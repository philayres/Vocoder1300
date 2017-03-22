/*
 * Copyright (C) 1993-2016 David Rowe
 *
 * All rights reserved
 * 
 * Licensed under GNU LGPL V2.1
 * See LICENSE file for information
 */
#include "defines.h"

static const float codes0[] = {
    225.0f,
    250.0f,
    275.0f,
    300.0f,
    325.0f,
    350.0f,
    375.0f,
    400.0f,
    425.0f,
    450.0f,
    475.0f,
    500.0f,
    525.0f,
    550.0f,
    575.0f,
    600.0f
};

static const float codes1[] = {
    325.0f,
    350.0f,
    375.0f,
    400.0f,
    425.0f,
    450.0f,
    475.0f,
    500.0f,
    525.0f,
    550.0f,
    575.0f,
    600.0f,
    625.0f,
    650.0f,
    675.0f,
    700.0f
};

static const float codes2[] = {
    500.0f,
    550.0f,
    600.0f,
    650.0f,
    700.0f,
    750.0f,
    800.0f,
    850.0f,
    900.0f,
    950.0f,
    1000.0f,
    1050.0f,
    1100.0f,
    1150.0f,
    1200.0f,
    1250.0f
};

static const float codes3[] = {
    700.0f,
    800.0f,
    900.0f,
    1000.0f,
    1100.0f,
    1200.0f,
    1300.0f,
    1400.0f,
    1500.0f,
    1600.0f,
    1700.0f,
    1800.0f,
    1900.0f,
    2000.0f,
    2100.0f,
    2200.0f
};

static const float codes4[] = {
    950.0f,
    1050.0f,
    1150.0f,
    1250.0f,
    1350.0f,
    1450.0f,
    1550.0f,
    1650.0f,
    1750.0f,
    1850.0f,
    1950.0f,
    2050.0f,
    2150.0f,
    2250.0f,
    2350.0f,
    2450.0f
};

static const float codes5[] = {
    1100.0f,
    1200.0f,
    1300.0f,
    1400.0f,
    1500.0f,
    1600.0f,
    1700.0f,
    1800.0f,
    1900.0f,
    2000.0f,
    2100.0f,
    2200.0f,
    2300.0f,
    2400.0f,
    2500.0f,
    2600.0f
};

static const float codes6[] = {
    1500.0f,
    1600.0f,
    1700.0f,
    1800.0f,
    1900.0f,
    2000.0f,
    2100.0f,
    2200.0f,
    2300.0f,
    2400.0f,
    2500.0f,
    2600.0f,
    2700.0f,
    2800.0f,
    2900.0f,
    3000.0f
};

static const float codes7[] = {
    2300.0f,
    2400.0f,
    2500.0f,
    2600.0f,
    2700.0f,
    2800.0f,
    2900.0f,
    3000.0f
};

static const float codes8[] = {
    2500.0f,
    2600.0f,
    2700.0f,
    2800.0f,
    2900.0f,
    3000.0f,
    3100.0f,
    3200.0f
};

static const float codes9[] = {
    2900.0f,
    3100.0f,
    3300.0f,
    3500.0f
};

const struct lsp_codebook lsp_cb[] = {
    {
        1,
        4,
        16,
        codes0
    },
    {
        1,
        4,
        16,
        codes1
    },
    {
        1,
        4,
        16,
        codes2
    },
    {
        1,
        4,
        16,
        codes3
    },
    {
        1,
        4,
        16,
        codes4
    },
    {
        1,
        4,
        16,
        codes5
    },
    {
        1,
        4,
        16,
        codes6
    },
    {
        1,
        3,
        8,
        codes7
    },
    {
        1,
        3,
        8,
        codes8
    },
    {
        1,
        2,
        4,
        codes9
    }
};
