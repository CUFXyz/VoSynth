#pragma once
#ifndef PHONEME_DB_H
#define PHONEME_DB_H
#include "oscillator.h"

typedef enum {
	VOWEL,
	NOISE,
    FRICATIVE,
    AFFRICATE,
	PLOSIVE,
	SONORANT
} PhonemeType;

void generate_vowel(short* output, int num_samples, int sample_rate, char vowel);
// Universal phoneme generator: handles vowels and consonants based on type and symbol
void generate_phoneme(short* output, int num_samples, int sample_rate, PhonemeType type, char symbol);
// Per-type generators (can be used directly or called from generate_phoneme)
void generate_fricative(short* output, int num_samples, int sample_rate, char symbol);
void generate_plosive(short* output, int num_samples, int sample_rate, char symbol);
void generate_consonant(short* output, int num_samples, int sample_rate, char symbol);
void generate_nasal(short* output, int num_samples, int sample_rate, char symbol);
void generate_noise(short* output, int num_samples, int sample_rate, char symbol);
void generate_affricate(short* output, int num_samples, int sample_rate, char symbol);
void generate_approximant(short* output, int num_samples, int sample_rate, char symbol);
void crossfade(short* dest, short* src1, short* src2, int overlap_samples);

#endif // !PHONEME_DB_H
