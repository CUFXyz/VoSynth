#include "adsr.h"

void envel_note_on(Envel* env)
{
	env->state = ATTACK;
	env->level = 0.0f;
}

void envel_note_off(Envel* env)
{
	env->state = RELEASE;
}

float envel_process(Envel* env)
{
	switch (env->state) {
	case ATTACK:
		env->level += env->attack_rate;
		if (env->level >= 1.0f) {
			env->level = 1.0f;
			env->state = DECAY;
		}
		break;
	case DECAY:
		env->level -= env->decay_rate;
		if (env->level <= env->sustain_level) {
			env->level = env->sustain_level;
			env->state = SUSTAIN;
		}
		break;
	case SUSTAIN:
		break;
	case RELEASE:
		env->level -= env->release_rate;
		if (env->level <= 0.0f) {
			env->level = 0.0f;
		}
		break;
	}
	return env->level;
}