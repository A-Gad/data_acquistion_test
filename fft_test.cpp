#include <stdio.h>
#include <math.h>
#include "kiss_fftr.h"

#define N 1024
#define SAMPLE_RATE 10000

int main() {
    // generate a 1000 Hz sine wave
    float samples[N];
    for(int i = 0; i < N; i++)
        samples[i] = sinf(2.0f * M_PI * 1000.0f * i / SAMPLE_RATE);

    // setup KissFFT
    kiss_fftr_cfg cfg = kiss_fftr_alloc(N, 0, NULL, NULL);
    kiss_fft_cpx output[N/2 + 1];

    // run FFT
    kiss_fftr(cfg, samples, output);

    // find peak frequency
    int peak_bin = 0;
    float peak_magnitude = 0;
    for(int i = 0; i < N/2 + 1; i++) {
        float mag = sqrtf(output[i].r * output[i].r + 
                          output[i].i * output[i].i);
        if(mag > peak_magnitude) {
            peak_magnitude = mag;
            peak_bin = i;
        }
    }

    float peak_freq = (float)peak_bin * SAMPLE_RATE / N;
    printf("Peak frequency: %.1f Hz (expected 1000 Hz)\n", peak_freq);

    kiss_fftr_free(cfg);
    return 0;
}