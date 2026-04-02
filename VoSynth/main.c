#include <stdio.h>
#include "src/VSCore.h"
#include "src/adsr.h"
#include "src/filter.h"
#include "src/oscillator.h"
#include "src/phoneme_db.h"

int main() {
	int sample_rate = 16000;
	int duration_ms = 2000;
	int num_samples = duration_ms * sample_rate / 1000;

	short* audio = (short*)malloc(num_samples * sizeof(short));
	if (!audio) {
		printf("Failed to alloc memory for audio\n");
		return 1;
	}

	generate_vowel(audio, num_samples, sample_rate, 'u');
	play_sound(audio, num_samples);

	free(audio);
	return 0;
}