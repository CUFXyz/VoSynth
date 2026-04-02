#pragma once
#ifndef ADSR_H
#define ADSR_H

typedef enum { ATTACK, DECAY, SUSTAIN, RELEASE } State;
typedef struct {
	State state;
	float level;
	float attack_rate;
	float decay_rate;
	float sustain_level;
	float release_rate;
} Envel;

void envel_note_on(Envel* env);
void envel_note_off(Envel* env);
float envel_process(Envel* env);

#endif // !ADSR_H
