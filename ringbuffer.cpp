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
    this->_buffer[this->write_index.load(std::memory_order_relaxed) & (this->capacity-1)];
    write_index.fetch_add(1, std::memory_order_release);
    
}

int RingBuffer::read(RingBuffer_entry* out)
{
    uint64_t current_write = write_index.load(std::memory_order_acquire);
    if( current_write == this->read_index.load(std::memory_order_relaxed) )
    {
        return -1;
    }
    if( current_write - this->read_index.load(std::memory_order_relaxed) > (this->capacity))
    {
        this->read_index.store(current_write - capacity, std::memory_order_relaxed);
    }
    *out = this->_buffer[this->read_index.load(std::memory_order_relaxed) & (capacity-1)];
    this->read_index.fetch_add(1, std::memory_order_release);
    return 0;
}

RingBuffer::~RingBuffer()
{
    delete[] _buffer;
}