#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/mman.h>
#include <atomic>
#include <memory>
#include <signal.h>
#include <unistd.h>
#include "ringbuffer.h"
#include "mcp3008spi.h"
#include "processing_thread.h"

std::atomic<bool> running{true};

struct thread_args
{
    std::shared_ptr<RingBuffer> buffer;
    mcp3008Spi* spidev;
    struct RingBuffer_entry* entry;
    struct timespec* ts;
    struct timespec* ts_wait;
};

void *thread_func(void *data)
{
    struct thread_args* args = static_cast<thread_args*>(data);
    clock_gettime(CLOCK_MONOTONIC, args->ts_wait);
    fprintf(stderr, "[RT thread] started\n");
    
    while(running)
    {
        int ret = 0;
        int32_t raw;
        ret = args->spidev->spi_Read(&raw, 0);
        if (ret == -1)
        {
            fprintf(stderr, "[RT thread] spi_Read failed\n");
            continue;
        }
        args->entry->spi_rx = static_cast<uint16_t>(raw);

        ret = clock_gettime(CLOCK_REALTIME,args->ts);
        if (ret == -1)
            break;
        args->entry->ts = *args->ts;
        args->buffer->write(args->entry);

        args->ts_wait->tv_nsec += 100000;  // 100µs = 10ksps
        if(args->ts_wait->tv_nsec >= 1000000000)
        {
            args->ts_wait->tv_sec++;
            args->ts_wait->tv_nsec -= 1000000000;
        }
        ret = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, args->ts_wait, NULL);
        if (ret != 0 && ret != EINTR)
            break;
    }
    fprintf(stderr, "[RT thread] exiting\n");
    return NULL;
}

void handler(int signum) {
    if(signum == SIGINT || signum == SIGTERM)
    {
        running = false;
    }
}

int main(int argc, char* argv[])
{
    struct sched_param param;
    pthread_attr_t attr;
    pthread_t thread;
    int ret;

    struct sigaction sa{};
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGINT, &sa, NULL) == -1)
        perror("sigaction SIGINT");
    if(sigaction(SIGTERM, &sa, NULL) == -1)
        perror("sigaction SIGTERM");

    /* Lock memory */
    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
        printf("mlockall failed: %m\n");
        exit(-2);
    }

    thread_args args{};
    args.buffer   = std::make_shared<RingBuffer>(4096); // Increased buffer size
    try
    {
        args.spidev   = new mcp3008Spi();
    }
    catch (const std::runtime_error& e)
    {
        printf("spi dev cannot be open: %s\n", e.what());
        return -1;
    }
    args.entry    = new RingBuffer_entry{};
    args.ts       = new timespec{};
    args.ts_wait  = new timespec{};

    /* Initialize pthread attributes */
    bool attr_initialised = false;
    ret = pthread_attr_init(&attr);
    if (ret) {
        printf("init pthread attributes failed\n");
        goto out;
    }
    attr_initialised = true;

    ret = pthread_attr_setstacksize(&attr, 4*PTHREAD_STACK_MIN);
    if (ret) {
        printf("pthread setstacksize failed\n");
        goto out;
    }

    ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    if (ret) {
        printf("pthread setschedpolicy failed\n");
        goto out;
    }
    param.sched_priority = 80;
    ret = pthread_attr_setschedparam(&attr, &param);
    if (ret) {
        printf("pthread setschedparam failed\n");
        goto out;
    }
    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (ret) {
        printf("pthread setinheritsched failed\n");
        goto out;
    }

    /* Start Sampling Thread */
    ret = pthread_create(&thread, &attr, thread_func, &args);
    if (ret) {
        printf("create pthread failed\n");
        goto out;
    }

    /* Start FFT Processing Thread */
    if (start_processing(args.buffer, running) != 0) {
        printf("failed to start processing thread\n");
        running = false;
    }

    /* Wait for shutdown */
    while (running) {
        sleep(1);
    }

    /* Cleanup */
    stop_processing();
    ret = pthread_join(thread, NULL);
    if (ret)
        printf("join pthread failed: %m\n");

out:
    if (attr_initialised)
        pthread_attr_destroy(&attr);
    delete args.spidev;
    delete args.ts;
    delete args.ts_wait;
    delete args.entry;
    return ret;
}
