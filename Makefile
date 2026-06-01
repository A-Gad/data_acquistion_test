CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
CXXFLAGS += -Wno-interference-size
LDFLAGS = -lpthread -lrt
TARGET = vibration-monitor
SRCS = aquistion.cpp mcp3008spi.cpp ringbuffer.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
