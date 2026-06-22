#include <gtest/gtest.h>
#include "ringbuffer.h"

TEST(RingBufferTest, BasicWriteRead) {
    RingBuffer rb(16);
    RingBuffer_entry entry;
    entry.spi_rx = 1234;
    entry.ts.tv_sec = 10;
    entry.ts.tv_nsec = 20;

    rb.write(&entry);
    
    RingBuffer_entry out;
    int result = rb.read(&out);
    
    EXPECT_EQ(result, 0);
    EXPECT_EQ(out.spi_rx, 1234);
    EXPECT_EQ(out.ts.tv_sec, 10);
    EXPECT_EQ(out.ts.tv_nsec, 20);
}

TEST(RingBufferTest, BufferEmpty) {
    RingBuffer rb(16);
    RingBuffer_entry out;
    int result = rb.read(&out);
    EXPECT_NE(result, 0);
}
