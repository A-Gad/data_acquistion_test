
#pragma once
#include <atomic>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <stdint.h>
#include <linux/types.h>
#include <new>
#include <time.h>

#ifdef __cpp_lib_hardware_interference_size
    using std::hardware_destructive_interference_size;
#else
    // 64 bytes on x86-64 │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ __cacheline_aligned │ ...
    constexpr std::size_t hardware_destructive_interference_size = 64;
#endif

struct RingBuffer_entry
{
    uint16_t spi_rx;
    struct timespec ts;
};

class RingBuffer
{
    private:
        alignas(hardware_destructive_interference_size) std::atomic<uint64_t> write_index{0};
        alignas(hardware_destructive_interference_size) std::atomic<uint64_t> read_index{0};

        struct RingBuffer_entry* _buffer;
        static constexpr const size_t DEFAULT_SIZE = 1024;
        size_t capacity;
        static size_t next_power_of_two(size_t n);
    public:
        RingBuffer();
        RingBuffer(size_t buff_size);
        void write(struct RingBuffer_entry* new_entry);
        int read(RingBuffer_entry* out);
        ~RingBuffer();
};

