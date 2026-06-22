#ifndef PROCESSING_THREAD_H
#define PROCESSING_THREAD_H

#include <pthread.h>
#include <atomic>
#include <memory>
#include "ringbuffer.h"
#include "inferenceEngine.h"

struct ProcessingResults {
    float shaft_fundamental;
    float fundamental_2x;
    float bearing_fault_max;
    float rms;
};

int start_processing(std::shared_ptr<RingBuffer> buffer, std::atomic<bool>& running);
void stop_processing();

#endif
