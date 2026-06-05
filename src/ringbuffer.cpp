#include "ringbuffer.h"
size_t RingBuffer::next_power_of_two(size_t n)
{
   size_t power = 1;
    while (power < n)
        power <<= 1;
    return power;
}

RingBuffer::RingBuffer() : RingBuffer(DEFAULT_SIZE){};

RingBuffer::RingBuffer(size_t buff_size)
{
    capacity = next_power_of_two(buff_size);
    _buffer = new RingBuffer_entry[capacity]();
}
void RingBuffer::write(struct RingBuffer_entry* new_entry)
{
    uint64_t curr_write = write_index.load(std::memory_order_relaxed);
    uint64_t curr_read  = read_index.load(std::memory_order_acquire); // Acquire ensures we read an accurate position

    // Check if buffer is completely full
    if ((curr_write - curr_read) >= capacity)
    {
        // Buffer is full! Drop the sample or log an overrun counter.
        return; 
    }

    // 1. Copy the data safely into the slots
    _buffer[curr_write & (capacity - 1)] = *new_entry;

    // 2. Advance the write pointer, releasing the data to the reading thread
    write_index.fetch_add(1, std::memory_order_release);
}

int RingBuffer::read(RingBuffer_entry* out)
{
    uint64_t curr_read  = read_index.load(std::memory_order_relaxed);
    uint64_t curr_write = write_index.load(std::memory_order_acquire); // Acquire ensures data is flushed to cache

    // Check if buffer is completely empty
    if (curr_write == curr_read)
    {
        return -1;
    }

    // 1. Fetch the data entry out of the slots
    *out = _buffer[curr_read & (capacity - 1)];

    // 2. Advance read pointer. Release semantics ensure we are done copying before the writer sees it's free.
    read_index.fetch_add(1, std::memory_order_release);
    return 0;
}

RingBuffer::~RingBuffer()
{
    delete[] _buffer;
}