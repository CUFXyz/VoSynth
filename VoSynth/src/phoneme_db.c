#include "phoneme_db.h"
#include "adsr.h"
#include "filter.h"

void generate_vowel(short* output, int num_samples, int sample_rate, char vowel)
{
	Oscillator osc = { .phase = 0, .frequency = 110.0f, .sample_rate = sample_rate };
	Envel env;
	env.attack_rate = 1.0f / (0.01f * sample_rate);
	env.decay_rate = 1.0f / (0.05f * sample_rate);
	env.sustain_level = 0.7f;
	env.release_rate = 1.0f / (0.1f * sample_rate);

	Biquad f1, f2;
	float formant1, formant2, gain1, gain2;

	switch (vowel) {
	case 'A': case 'a':
		formant1 = 700; formant2 = 1150; gain1 = 15; gain2 = 10; break;
	case 'O': case 'o':
		formant1 = 550; formant2 = 1000; gain1 = 12; gain2 = 8; break;
	case 'I': case 'i':
		formant1 = 300; formant2 = 2200; gain1 = 10; gain2 = 12; break;
	case 'E': case 'e':
		formant1 = 450; formant2 = 1800; gain1 = 12; gain2 = 10; break;
	case 'U': case 'u':
		formant1 = 350; formant2 = 800; gain1 = 10; gain2 = 8; break;
	default:
		formant1 = 700; formant2 = 1150; gain1 = 15; gain2 = 10;
	}

	biquad_peak(&f1, formant1, 10, gain1, (float)sample_rate);
	biquad_peak(&f2, formant2, 10, gain2, (float)sample_rate);

	envel_note_on(&env);

	for (int i = 0; i < num_samples; i++) {
		float sample = oscillator_saw(&osc);
		sample = biquad_process(&f1, sample);
		sample = biquad_process(&f2, sample);
		float gain = envel_process(&env);
		sample *= gain;

		if (sample > 1.0f) sample = 1.0f;
		if (sample < -1.0f) sample = -1.0f;
		output[i] = (short)(sample * 32767);

		if (i == num_samples / 2) envel_note_off(&env);
	}
}

void crossfade(short* dest, short* src1, short* src2, int overlap_samples)
{
	for (int i = 0; i < overlap_samples; i++) {
		float t = (float)i / overlap_samples;
		dest[i] = (short)(src1[i] * (1.0f - t) + src2[i] * t);
	}
}
