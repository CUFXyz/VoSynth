#include <stdio.h>
#include "src/VSCore.h"
#include "src/adsr.h"
#include "src/filter.h"
#include "src/oscillator.h"
#include "src/phoneme_db.h"

PhonemeType detect_type(char c)
{
	switch (tolower((unsigned char)c)) {
	case 'a': case 'e': case 'i': case 'o': case 'u':
		return VOWEL;
	case 's': case 'z': case 'f': case 'v': case 'h':
		return FRICATIVE;
	case 'p': case 't': case 'k': case 'b': case 'd': case 'g':
		return PLOSIVE;
	case 'm': case 'n': case 'r': case 'l':
		return SONORANT;
	default:
		return NOISE;
	}
}

int main() {
	int sample_rate = 16000;
	int duration_ms = 1000;
	int num_samples = duration_ms * sample_rate / 1000;
	char vowel_input;
	

	while (1) {
		printf("\n> ");
		scanf_s(" %c", &vowel_input, 1);

		if (vowel_input == '0') {
			break;
		}

		PhonemeType type = detect_type(vowel_input);

		short* audio = (short*)malloc(num_samples * sizeof(short));
		if (!audio) {
			printf("Failed to alloc memory for audio\n");
			continue;
		}

		generate_phoneme(audio, num_samples, sample_rate, type, vowel_input);
		play_sound(audio, num_samples);
		free(audio);
	}

	return 0;
}