#include "oscillator.h"


float oscillator_saw(Oscillator* osc)
{
	float value = 1.0f - 2.0f * osc->phase;
	osc->phase += osc->frequency / osc->sample_rate;
	if (osc->phase >= 1.0f) osc->phase -= 1.0f;
	return value;
}

float oscillator_sin(Oscillator* osc)
{
	float value = sinf(2.0f * M_PI * osc->phase);
	osc->phase += osc->frequency / osc->sample_rate;
	if (osc->phase >= 1.0f) osc->phase -= 1.0f;
	return value;
}

void oscillator_reset(Oscillator* osc) {
	osc->phase = 0.0f;
}