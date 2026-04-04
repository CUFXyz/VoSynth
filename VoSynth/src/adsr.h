#pragma once
#ifndef ADSR_H
#define ADSR_H

#include <stdbool.h>

/*
 ADSR - Attack, Decay, Sustain, Release
 
 Envel is a curve that changes sound volume over time.

 Envel states
 
 IDLE - Waiting, sound is inactive (level = 0)
 ATTACK - Attack phase: fast rise from 0 to 1
 DECAY - Decay phase: decrease from 1 to sustain_level
 SUSTAIN - Sustain phase: stable level
 RELEASE - Release phase: fall from sustain_level to 0
*/
typedef enum { IDLE, ATTACK, DECAY, SUSTAIN, RELEASE } State;

/*
 ADSR envel structure
 
 Each sample we add or substract a fixed value (rate)
 to make the level change smoothly
*/
typedef struct {
	State state; // Current envel phase
	float level; // current volume level (0.0 - 1.0)
	float attack_rate; // Level increment per sample during attack
	float decay_rate; // Level decrement per sample during decay
	float sustain_level; // Sustain level
	float release_rate; // Level decrement per sample during release
	bool is_active; // Activity flag
} Envel;

/*
	Initialize envel with time parameters (in sec)

	env - Pointer to envel structure
	attack_s - Attack duration in seconds (0.01 = 10ms)
	decay_s - Decay duration in seconds (0.05 = 50ms)
	sustain_level - Sustain level (0.0-1.0)
	release_s - Release duration in seconds (0.1 = 100ms)
	sample_rate - Sample rate in Hz
*/
void envel_init(Envel* env, float attack_s, float decay_s, float sustain_level, float release_s, int sample_rate);

/*
	Start a note

	Transitions envel to attack phase. If envel was in IDLE, start from zero.
	If already sounding - smoothly transition to attack
*/
void envel_note_on(Envel* env);

/*
	Stops a note

	Transitions envel to release phase (RELEASE).
	Level will start falling from current value to zero.
*/
void envel_note_off(Envel* env);

/*
	Process one sample of the envel

	Must be called for each generated sound sample.
	Returns current volume level (0.0-1.0) that should be multiplied by the generated sample.
*/
float envel_process(Envel* env);

/*
	Checks if envel is active

	return true if level > 0 or fading out, false if IDLE
*/
bool envel_is_active(const Envel* env);

#endif // !ADSR_H