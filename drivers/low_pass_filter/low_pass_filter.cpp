#include <cmath>
#include <pico_drivers/low_pass_filter/low_pass_filter.hpp>

#define LPF_DEBUG (false)

#if LPF_DEBUG
#include <stdio.h>
#endif

LowPassFilter::LowPassFilter() : output(0.0f), ePow(0.0f) {}

LowPassFilter::LowPassFilter(float cut_off_freq, float delta_time)
    : output(0), ePow(1 - exp(-delta_time * 2.0f * static_cast<float>(M_PI) * cut_off_freq)) {
#if LPF_DEBUG
    if (delta_time <= 0) {
        printf("Warning: LowPassFilter instance was configured with 0s as delta time\n");
        ePow = 0.0f;
    }

    if (cut_off_freq <= 0) {
        printf("Warning: LowPassFilter instance was configured with 0Hz cut-off frequency\n");
        ePow = 0.0f;
    }
#endif
}

float LowPassFilter::update(float input) { return output += (input - output) * ePow; }

float LowPassFilter::update(float input, float delta_time, float cut_off_freq) {
    reconfigure_filter(delta_time, cut_off_freq);
    return update(input);
}

void LowPassFilter::reconfigure_filter(float delta_time, float cut_off_freq) {
#if LPF_DEBUG
    if (delta_time <= 0 || cut_off_freq <= 0) {
        printf("Warning: A LowPassFilter instance was configured incorrectly.\n");
        ePow = 0.0f;
        return;
    }
#endif
    ePow = 1 - exp(-delta_time * 2 * static_cast<float>(M_PI) * cut_off_freq);
}

float LowPassFilter::get_output() const { return output; }
