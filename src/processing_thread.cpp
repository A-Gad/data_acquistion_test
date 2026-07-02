#include "processing_thread.h"
#include "kiss_fftr.h"
#include <vector>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <unistd.h>

static constexpr size_t FFT_SIZE = 1024;
static pthread_t thread_id;
static bool thread_started = false;

struct ThreadContext {
    std::shared_ptr<RingBuffer> buffer;
    std::atomic<bool>* running;
    std::unique_ptr<InferenceEngine> inference_engine = nullptr;
};

static void* processing_thread_func(void* data) {
    ThreadContext* ctx = static_cast<ThreadContext*>(data);
    
    kiss_fftr_cfg cfg = kiss_fftr_alloc(FFT_SIZE, 0, nullptr, nullptr);
    if (!cfg) {
        std::cerr << "[Processing] Failed to allocate KissFFT config\n";
        delete ctx;
        return nullptr;
    }

    std::vector<float> time_samples(FFT_SIZE);
    std::vector<kiss_fft_cpx> freq_out(FFT_SIZE / 2 + 1);
    std::vector<float> magnitude(FFT_SIZE / 2 + 1);

    std::cout << "[Processing] Thread started\n";

    while (ctx->running->load()) {
        for (size_t i = 0; i < FFT_SIZE; ++i) {
            RingBuffer_entry entry;
            while (ctx->buffer->read(&entry) == -1) {
                if (!ctx->running->load()) goto cleanup;
                usleep(1000);
            }
            
            float window = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (FFT_SIZE - 1)));
            time_samples[i] = (static_cast<float>(entry.spi_rx) - 512.0f) * window;
        }

        kiss_fftr(cfg, time_samples.data(), freq_out.data());

        float sum_sq = 0;
        for (size_t i = 0; i <= FFT_SIZE / 2; ++i) {
            magnitude[i] = sqrtf(freq_out[i].r * freq_out[i].r + freq_out[i].i * freq_out[i].i) / (FFT_SIZE / 2.0f);
            sum_sq += magnitude[i] * magnitude[i];
        }
        float rms = sqrtf(sum_sq / (FFT_SIZE / 2.0f + 1));

        float shaft_fundamental = magnitude[22];
        float fundamental_2x = magnitude[44];

        float bearing_fault_max = 0;
        for (size_t i = 66; i <= 173; ++i) {
            if (magnitude[i] > bearing_fault_max) {
                bearing_fault_max = magnitude[i];
            }
        }
        
        // if(!(ctx->inference_engine->loadModel(MODEL_PATH)))
        // {
        //     std::cerr<<"cannot load model!" ;
        //     !ctx->running;
        // }


        std::cout << std::fixed << std::setprecision(2)
                  << "\r[FFT] RMS: " << rms 
                  << " | Shaft: " << shaft_fundamental 
                  << " | 2x: " << fundamental_2x 
                  << " | Bearing: " << bearing_fault_max << "    " << std::flush;
    }

cleanup:
    std::cout << "\n[Processing] Thread exiting\n";
    kiss_fft_free(cfg);
    delete ctx;
    return nullptr;
}

int start_processing(std::shared_ptr<RingBuffer> buffer, std::atomic<bool>& running) {
    if (thread_started) return -1;

    ThreadContext* ctx = new ThreadContext{buffer, &running};
    int ret = pthread_create(&thread_id, nullptr, processing_thread_func, ctx);
    if (ret == 0) {
        thread_started = true;
    } else {
        delete ctx;
    }
    return ret;
}

void stop_processing() {
    if (thread_started) {
        pthread_join(thread_id, nullptr);
        thread_started = false;
    }
}
