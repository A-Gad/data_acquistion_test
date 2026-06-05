CXX ?= g++
CC ?= gcc
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
CXXFLAGS += -Wno-interference-size
CFLAGS ?= -O2 -Wall
LDFLAGS = -lpthread -lrt -lm

TARGET = vibration-monitor

CPP_SRCS = aquistion.cpp mcp3008spi.cpp ringbuffer.cpp processing_thread.cpp
C_SRCS = kiss_fft.c kiss_fftr.c
OBJS = $(CPP_SRCS:.cpp=.o) $(C_SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

fft_test: fft_test.cpp kiss_fft.o kiss_fftr.o
	$(CXX) $(CXXFLAGS) -o fft_test fft_test.cpp kiss_fft.o kiss_fftr.o -lm

clean:
	rm -f $(OBJS) $(TARGET) fft_test

.PHONY: all clean
