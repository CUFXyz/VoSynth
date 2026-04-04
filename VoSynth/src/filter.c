#include "filter.h"

void biquad_peak(Biquad* f, float freq, float Q, float gain_db, float sample_rate)
{

    if (!f || sample_rate <= 0 || freq <= 0 || Q <= 0) return;

    if (freq >= sample_rate * 0.45f) freq = sample_rate * 0.45f;

    float omega = 2.0f * M_PI * freq / sample_rate;
    float sin_omega = sinf(omega);
    float cos_omega = cosf(omega);
    float alpha = sin_omega / (2.0f * Q);
    float A = powf(10.0f, gain_db / 40.0f);

    f->b0 = 1.0f + alpha * A;
    f->b1 = -2.0f * cos_omega;
    f->b2 = 1.0f - alpha * A;
    float a0 = 1.0f + alpha / A;
    f->a1 = -2.0f * cos_omega;
    f->a2 = 1.0f - alpha / A;


    float inv_a0 = 1.0f / a0;
    f->b0 *= inv_a0;
    f->b1 *= inv_a0;
    f->b2 *= inv_a0;
    f->a1 *= inv_a0;
    f->a2 *= inv_a0;

    f->x1 = f->x2 = f->y1 = f->y2 = 0.0f;
}

void biquad_lowpass(Biquad* f, float cutoff, float sample_rate)
{
    float omega = 2.0f * M_PI * cutoff / sample_rate;
    float sin_omega = sinf(omega);
    float cos_omega = cosf(omega);
    float alpha = sin_omega / (2.0f * 0.707f);

    float b0 = (1.0f - cos_omega) / 2.0f;
    float b1 = 1.0f - cos_omega;
    float b2 = (1.0f - cos_omega) / 2.0f;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * cos_omega;
    float a2 = 1.0f - alpha;

    f->b0 = b0 / a0;
    f->b1 = b1 / a0;
    f->b2 = b2 / a0;
    f->a1 = a1 / a0;
    f->a2 = a2 / a0;

    f->x1 = f->x2 = f->y1 = f->y2 = 0.0f;
}

float biquad_process(Biquad* f, float input)
{
    float output = f->b0 * input + f->b1 * f->x1 + f->b2 * f->x2
        - f->a1 * f->y1 - f->a2 * f->y2;

    f->x2 = f->x1;
    f->x1 = input;
    f->y2 = f->y1;
    f->y1 = output;

    return output;
}