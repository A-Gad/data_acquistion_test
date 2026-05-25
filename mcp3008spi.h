#ifndef MCP3008SPI_H
    #define MCP3008SPI_H
     
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include "ringbuffer.h"


class mcp3008Spi{
 
public:
    mcp3008Spi();
    mcp3008Spi(const std::string& devspi, uint8_t spiMode,  uint32_t spiSpeed, uint8_t spibitsPerWord);
    ~mcp3008Spi();
    int spiWriteRead( int32_t *data, int length);
    int spi_Read(int32_t *buff, uint8_t channel);
    

private:
    static constexpr const char* DEFAULT_DEVICE = "/dev/spidev0.0";
    static constexpr uint32_t DEFAULT_SPEED = 1000000;
    static constexpr uint8_t DEFAULT_BITS = 8;
    uint8_t _mode;
    uint8_t _bitsPerWord;
    uint32_t  _speed;
    int _spifd;
    int spiOpen(const char* devspi);
    int spiClose();
     
};
 
#endif