#include "phoneme_db.h"
#include "adsr.h"
#include "filter.h"
#include <stdlib.h>

#define ATTACK_MS 10 / 1000.0f
#define DECAY_MS 50 / 1000.0f
#define RELEASE_MS 100 / 1000.0f
#define SUSTAIN_LEVEL 0.7f

// Simple white noise helper at file scope
static inline float white_noise(void) { return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f; }

void generate_vowel(short* output, int num_samples, int sample_rate, char vowel)
{
	Oscillator osc = { .phase = 0, .frequency = 120.0f, .sample_rate = (float)sample_rate };

	Envel env;
	envel_init(&env,
		ATTACK_MS,
		DECAY_MS,
		SUSTAIN_LEVEL,
		RELEASE_MS,
		sample_rate);

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

// Universal phoneme generator: vowels are delegated to generate_vowel,
// consonants are synthesized using noise, filters and optional voiced source.
void generate_phoneme(short* output, int num_samples, int sample_rate, PhonemeType type, char symbol)
{
    if (type == VOWEL) {
		generate_vowel(output, num_samples, sample_rate, symbol);
		return;
	}

	// dispatch to per-type generators
	switch (type) {
 case FRICATIVE:
		generate_fricative(output, num_samples, sample_rate, symbol);
		break;
    case AFFRICATE:
		generate_affricate(output, num_samples, sample_rate, symbol);
		break;
	case NOISE:
		generate_noise(output, num_samples, sample_rate, symbol);
		break;
	case PLOSIVE:
		generate_plosive(output, num_samples, sample_rate, symbol);
		break;
	case SONORANT:
		// sonorants include nasals and voiced consonants - try nasals first
		if (symbol == 'M' || symbol == 'm' || symbol == 'N' || symbol == 'n')
			generate_nasal(output, num_samples, sample_rate, symbol);
		else
			generate_consonant(output, num_samples, sample_rate, symbol);
		break;
	default:
		generate_noise(output, num_samples, sample_rate, symbol);
		break;
	}
}

// Per-type implementations
void generate_fricative(short* output, int num_samples, int sample_rate, char symbol)
{
    Biquad band, lp;
	Oscillator osc = { .phase = 0, .frequency = 120.0f, .sample_rate = (float)sample_rate };
	Envel env; 
	envel_init(&env, 0.005f, 0.02f, 0.8f, 0.05f, sample_rate);
	// choose parameters per symbol
	if (symbol == 'S' || symbol == 's') {
		biquad_peak(&band, 6000.0f, 5.0f, 6.0f, (float)sample_rate);
	}
	else if (symbol == 'Z' || symbol == 'z') {
		biquad_peak(&band, 5500.0f, 4.5f, 5.0f, (float)sample_rate);
	}
	else if (symbol == 'F' || symbol == 'f') {
		biquad_peak(&band, 3500.0f, 3.0f, 4.0f, (float)sample_rate);
	}
	else if (symbol == 'V' || symbol == 'v') {
		biquad_peak(&band, 3500.0f, 3.0f, 4.0f, (float)sample_rate);
	}
	else if (symbol == 'H' || symbol == 'h') {
		// broadband breathy noise
		biquad_lowpass(&lp, 8000.0f, (float)sample_rate);
	}
	else {
		biquad_peak(&band, 4000.0f, 3.0f, 2.0f, (float)sample_rate);
	}

	envel_note_on(&env);
	for (int i = 0; i < num_samples; i++) {
		float n = white_noise();
		if (symbol == 'H' || symbol == 'h') {
			n = biquad_process(&lp, n);
		} else {
			n = biquad_process(&band, n);
		}
		float g = envel_process(&env);
		float sample = n * g;
		// add voiced component for voiced fricatives
		if (symbol == 'Z' || symbol == 'z' || symbol == 'V' || symbol == 'v') {
			float v = oscillator_sin(&osc) * 0.5f * g;
			sample = sample * 0.6f + v * 0.4f;
		} else sample *= 0.65f;

		if (sample > 1.0f) sample = 1.0f;
		if (sample < -1.0f) sample = -1.0f;
		output[i] = (short)(sample * 32767);
		if (i == num_samples / 2) envel_note_off(&env);
	}
}

void generate_plosive(short* output, int num_samples, int sample_rate, char symbol)
{
    // More realistic plosive: shaped multi-band burst + modulated aspiration
	Biquad burst1, burst2, burst3, asp_lp;
	Oscillator osc = { .phase = 0, .frequency = 110.0f, .sample_rate = (float)sample_rate };

	// Envelopes
	Envel env_burst; envel_init(&env_burst, 0.0008f, 0.012f, 0.0f, 0.01f, sample_rate);
	Envel env_asp; envel_init(&env_asp, 0.004f, 0.03f, 0.0f, 0.03f, sample_rate);

	// base center freq
	float cf = 3000.0f;
	if (symbol == 'P' || symbol == 'p') cf = 2800.0f;
	else if (symbol == 'T' || symbol == 't') cf = 4200.0f;
	else if (symbol == 'K' || symbol == 'k') cf = 2200.0f;
	else if (symbol == 'B' || symbol == 'b' || symbol == 'D' || symbol == 'd' || symbol == 'G' || symbol == 'g') cf = 3000.0f;

	// three bands to create burst spectral shape
	biquad_peak(&burst1, cf * 0.6f, 2.0f, 9.0f, (float)sample_rate);
	biquad_peak(&burst2, cf, 2.4f, 10.0f, (float)sample_rate);
	biquad_peak(&burst3, cf * 1.5f, 3.5f, 6.0f, (float)sample_rate);
	biquad_lowpass(&asp_lp, 7000.0f, (float)sample_rate);

	int burst_ms = 10;
	int burst_samples = (sample_rate * burst_ms) / 1000;
	if (burst_samples > num_samples) burst_samples = num_samples;

	// burst phase
	envel_note_on(&env_burst);
	for (int i = 0; i < burst_samples; i++) {
		float n = white_noise();
		float b = 0.0f;
		b += biquad_process(&burst1, n) * 0.9f;
		b += biquad_process(&burst2, n) * 1.0f;
		b += biquad_process(&burst3, n) * 0.6f;

		// apply a short cosine window to reduce click
		float win = sinf((float)M_PI * (i + 1) / (float)(burst_samples + 1));

		float g = envel_process(&env_burst) * win;
		float sample = b * g * 1.1f;

		// small voiced component for voiced plosives
		if (symbol == 'B' || symbol == 'b' || symbol == 'D' || symbol == 'd' || symbol == 'G' || symbol == 'g') {
			float v = oscillator_sin(&osc) * 0.35f * g;
			sample = sample * 0.6f + v * 0.4f;
		}
		if (sample > 1.0f) sample = 1.0f;
		if (sample < -1.0f) sample = -1.0f;
		output[i] = (short)(sample * 32767);
		if (i == burst_samples / 2) envel_note_off(&env_burst);
	}

	// aspiration phase with turbulence modulation
	if (burst_samples < num_samples) {
		envel_note_on(&env_asp);
		static float turb = 0.0f;
		for (int i = burst_samples; i < num_samples; i++) {
			float n = white_noise();
			// turbulent amplitude modulation
			float r = fabsf(white_noise());
			turb += (r - turb) * 0.03f;
			float air = 0.4f + 0.8f * turb; // 0.4..1.2

			float a = biquad_process(&asp_lp, n) * 0.5f * air;
			float g = envel_process(&env_asp);
			float sample = a * g;
			if (symbol == 'B' || symbol == 'b' || symbol == 'D' || symbol == 'd' || symbol == 'G' || symbol == 'g') {
				float v = oscillator_sin(&osc) * 0.25f * g;
				sample = sample * 0.5f + v * 0.5f;
			}
			if (sample > 1.0f) sample = 1.0f;
			if (sample < -1.0f) sample = -1.0f;
			output[i] = (short)(sample * 32767);
			if (i == burst_samples + (sample_rate * 0.02)) envel_note_off(&env_asp);
		}
	}
}

void generate_consonant(short* output, int num_samples, int sample_rate, char symbol)
{
    Oscillator osc = { .phase = 0, .frequency = 120.0f, .sample_rate = (float)sample_rate };
	Biquad band, low, f1, f2;
	Envel env;
	envel_init(&env, 0.005f, 0.02f, 0.8f, 0.05f, sample_rate);

	if (symbol == 'Z' || symbol == 'z') {
		biquad_peak(&band, 5500.0f, 4.5f, 5.0f, (float)sample_rate);
		biquad_lowpass(&low, 1500.0f, (float)sample_rate);
	} else if (symbol == 'R' || symbol == 'r' || symbol == 'L' || symbol == 'l') {
		// liquids: shape with low formants
		biquad_peak(&f1, 300.0f, 1.0f, 6.0f, (float)sample_rate);
		biquad_peak(&f2, 1600.0f, 1.5f, 4.0f, (float)sample_rate);
	} else {
		biquad_lowpass(&low, 1500.0f, (float)sample_rate);
		biquad_peak(&band, 2000.0f, 4.0f, 6.0f, (float)sample_rate);
	}

	envel_note_on(&env);
	for (int i = 0; i < num_samples; i++) {
		float v = oscillator_sin(&osc) * 0.6f;
		float n = white_noise() * 0.35f;
		if (symbol == 'R' || symbol == 'r' || symbol == 'L' || symbol == 'l') {
			float shaped = biquad_process(&f1, v);
			shaped += biquad_process(&f2, v) * 0.4f;
			float g = envel_process(&env);
			float sample = shaped * g * 0.9f;
			if (sample > 1.0f) sample = 1.0f;
			if (sample < -1.0f) sample = -1.0f;
			output[i] = (short)(sample * 32767);
		} else {
			n = biquad_process(&band, n);
			float mixed = v + n;
			mixed = biquad_process(&low, mixed);
			float g = envel_process(&env);
			float sample = mixed * g;
			if (sample > 1.0f) sample = 1.0f;
			if (sample < -1.0f) sample = -1.0f;
			output[i] = (short)(sample * 32767);
		}
		if (i == num_samples / 2) envel_note_off(&env);
	}
}

void generate_nasal(short* output, int num_samples, int sample_rate, char symbol)
{
	Oscillator osc = { .phase = 0, .frequency = 120.0f, .sample_rate = (float)sample_rate };
	Biquad f1, f2;
    if (symbol == 'N' || symbol == 'n') {
		biquad_peak(&f1, 300.0f, 1.2f, 7.0f, (float)sample_rate);
		biquad_peak(&f2, 1400.0f, 1.5f, 3.5f, (float)sample_rate);
	} else {
		biquad_peak(&f1, 250.0f, 1.2f, 8.0f, (float)sample_rate);
		biquad_peak(&f2, 1200.0f, 1.5f, 3.0f, (float)sample_rate);
	}
	Envel env; 
	envel_init(&env, 0.005f, 0.02f, 0.8f, 0.05f, sample_rate);
	envel_note_on(&env);
	for (int i = 0; i < num_samples; i++) {
		float v = oscillator_sin(&osc) * 0.9f;
		float shaped = biquad_process(&f1, v);
		shaped += biquad_process(&f2, v) * 0.3f;
		float g = envel_process(&env);
		float sample = shaped * g * 0.8f;
		if (sample > 1.0f) sample = 1.0f;
		if (sample < -1.0f) sample = -1.0f;
		output[i] = (short)(sample * 32767);
		if (i == num_samples / 2) envel_note_off(&env);
	}
}

void generate_noise(short* output, int num_samples, int sample_rate, char symbol)
{
	Biquad band;
	Envel env; 
	envel_init(&env, 0.005f, 0.02f, 0.8f, 0.05f, sample_rate);
	biquad_peak(&band, 1500.0f, 1.0f, 0.0f, (float)sample_rate);
	envel_note_on(&env);
	for (int i = 0; i < num_samples; i++) {
		float n = white_noise();
		n = biquad_process(&band, n);
		float g = envel_process(&env);
		float sample = n * g * 0.5f;
		if (sample > 1.0f) sample = 1.0f;
		if (sample < -1.0f) sample = -1.0f;
		output[i] = (short)(sample * 32767);
		if (i == num_samples / 2) envel_note_off(&env);
	}
}

void generate_affricate(short* output, int num_samples, int sample_rate, char symbol)
{
	// affricates = plosive burst followed by fricative
	int half = num_samples / 2;
	generate_plosive(output, half, sample_rate, symbol == 'C' ? 'T' : symbol);
	generate_fricative(output + half, num_samples - half, sample_rate, symbol == 'C' ? 'S' : symbol);
}

void generate_approximant(short* output, int num_samples, int sample_rate, char symbol)
{
	// gentle voiced resonant sounds (w, j)
	Oscillator osc = { .phase = 0, .frequency = 180.0f, .sample_rate = (float)sample_rate };
	Biquad low, f1;
	biquad_lowpass(&low, 2000.0f, (float)sample_rate);
	biquad_peak(&f1, 800.0f, 1.0f, 4.0f, (float)sample_rate);
	Envel env; 
	envel_init(&env, 0.01f, 0.02f, 0.8f, 0.05f, sample_rate);
	envel_note_on(&env);
	for (int i = 0; i < num_samples; i++) {
		float v = oscillator_sin(&osc) * 0.7f;
		float s = biquad_process(&low, v);
		s = biquad_process(&f1, s);
		float g = envel_process(&env);
		float sample = s * g * 0.9f;
		if (sample > 1.0f) sample = 1.0f;
		if (sample < -1.0f) sample = -1.0f;
		output[i] = (short)(sample * 32767);
		if (i == num_samples / 2) envel_note_off(&env);
	}
}