#include "adsr.h"

void envel_init(Envel* env, float attack_s, float decay_s, float sustain_level, float release_s, int sample_rate)
{
	if (!env) return; // Catching nullptr

	/*
		Convert time in seconds to increment per sample

		Formula: rate = 1.0 / (duration_in_samples)
		duration_in_samples = seconds * sample_rate
	*/
	env->attack_rate = (attack_s > 0.0f) ? 1.0f / (attack_s * sample_rate) : 1.0f;
	env->decay_rate = (decay_s > 0.0f) ? 1.0f / (decay_s * sample_rate) : 1.0f;
	env->release_rate = (release_s > 0.0f) ? 1.0f / (release_s * sample_rate) : 1.0f;
	env->sustain_level = sustain_level;

	env->state = IDLE;
	env->level = 0.0f;
	env->is_active = false;
}

void envel_note_on(Envel* env)
{
	if (!env) return;

	if (env->state == IDLE || env->level <= 0.0f) {
		env->level = 0.0f;
		env->state = ATTACK;
	}
	else {
		env->state = ATTACK;
	}
	env->is_active = true;
}

void envel_note_off(Envel* env)
{
	if (!env || env->state == IDLE) return;

	env->state = RELEASE;
}

float envel_process(Envel* env)
{
	if (!env || !env->is_active) return 0.0f;

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
			env->state = IDLE;
			env->is_active = false;
		}
		break;
	case IDLE:
	default:
		return 0.0f;
	}
	return env->level;
}

bool envel_is_active(const Envel* env)
{
	return env && env->is_active;
}
