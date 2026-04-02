#pragma once

#ifndef FILTER_H
#define FILTER_H
#define _USE_MATH_DEFINES
#include <math.h>

typedef struct {
    float b0, b1, b2, a1, a2;
    float x1, x2, y1, y2;
} Biquad;

void biquad_peak(Biquad* f, float freq, float Q, float gain_db, float sample_rate);
float biquad_process(Biquad* f, float input);

#endif // !FILTER_H


