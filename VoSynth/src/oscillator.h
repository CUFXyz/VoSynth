#pragma once

#ifndef OSCILLATOR_H
#define OSCILLATOR_H
#define _USE_MATH_DEFINES
#include <math.h>


typedef struct {
	float phase;
	float frequency;
	float sample_rate;
} Oscillator;

float oscillator_saw(Oscillator* osc);
float oscillator_sin(Oscillator* osc);
void oscillator_reset(Oscillator* osc);

#endif // !OSCILLATOR_H