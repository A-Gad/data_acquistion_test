#include <gtest/gtest.h>
#include <math.h>
#include "kiss_fftr.h"

TEST(SignalProcessing, PeakFrequencyDetection) {
    const int N = 1024;
    const int SAMPLE_RATE = 10000;
    float samples[N];
    for(int i = 0; i < N; i++)
        samples[i] = sinf(2.0f * M_PI * 1000.0f * i / SAMPLE_RATE);

    kiss_fftr_cfg cfg = kiss_fftr_alloc(N, 0, NULL, NULL);
    ASSERT_NE(cfg, nullptr);
    
    kiss_fft_cpx output[N/2 + 1];
    kiss_fftr(cfg, samples, output);

    int peak_bin = 0;
    float peak_magnitude = 0;
    for(int i = 0; i < N/2 + 1; i++) {
        float mag = sqrtf(output[i].r * output[i].r + output[i].i * output[i].i);
        if(mag > peak_magnitude) {
            peak_magnitude = mag;
            peak_bin = i;
        }
    }

    float peak_freq = (float)peak_bin * SAMPLE_RATE / N;
    EXPECT_NEAR(peak_freq, 1000.0, 10.0);

    kiss_fftr_free(cfg);
}
