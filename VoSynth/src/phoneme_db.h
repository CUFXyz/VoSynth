#pragma once
#ifndef PHONEME_DB_H
#define PHONEME_DB_H
#include "oscillator.h"

void generate_vowel(short* output, int num_samples, int sample_rate, char vowel);
void crossfade(short* dest, short* src1, short* src2, int overlap_samples);

#endif // !PHONEME_DB_H
