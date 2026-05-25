#include "mcp3008spi.h"


mcp3008Spi::mcp3008Spi() : mcp3008Spi(DEFAULT_DEVICE, SPI_MODE_0, 
                               DEFAULT_SPEED, DEFAULT_BITS) 
{}

mcp3008Spi::mcp3008Spi(const std::string& devspi, uint8_t spiMode,  uint32_t spiSpeed, uint8_t spibitsPerWord)
{
    _mode = spiMode;
    _speed = spiSpeed;
    _bitsPerWord = spibitsPerWord;
    if((this->spiOpen(devspi.c_str())) == -1)
    {
        throw std::runtime_error("Failed to open SPI device");
       
    }
}

int mcp3008Spi::spiOpen(const char* devspi)
{
    int status = -1;
    _spifd = open(devspi, O_RDWR);
    if(_spifd == -1)
    {
        perror("open");
        return -1;
    }
    status = ioctl(_spifd, SPI_IOC_WR_MODE, &_mode);
    if(status == -1)
    {
        perror("ioctl -> cannot set the mode");
        return -1;
    }
    status = ioctl(_spifd, SPI_IOC_WR_BITS_PER_WORD, &_bitsPerWord);
    if(status == -1)
    {
        perror("ioctl -> cannot set wr bits per word");
        return -1;
    }
    status = ioctl(_spifd, SPI_IOC_RD_BITS_PER_WORD, &_bitsPerWord);
    if(status == -1)
    {
        perror("ioctl -> cannot set rd bits per word");
        return -1;
    }
    status = ioctl(_spifd, SPI_IOC_WR_MAX_SPEED_HZ, &_speed);
    if (status == -1)
    {
        perror("ioctl -> cannot set max wr speed");
        return -1;
    }
    status = ioctl(_spifd, SPI_IOC_RD_MAX_SPEED_HZ, &_speed);
    if (status == -1)
    {
        perror("ioctl -> cannot set max rd speed");
        return -1;
    }  
    return status;
}

int mcp3008Spi::spiClose()
{
    int status = -1;
    status = close(_spifd);
    if(status == -1)
    {
        perror("could not close the spi device");
        return -1;
    }
    return status;
}
int mcp3008Spi::spiWriteRead( int32_t *data, int length)
{
    std::vector<struct spi_ioc_transfer> spi(length);
    int retval = -1;
    for (int i = 0; i < length; ++i)
    {
        spi[i].tx_buf        = (unsigned long)(data + i); 
        spi[i].rx_buf        = (unsigned long)(data + i); 
        spi[i].len           = sizeof(*(data + i)) ;
        spi[i].delay_usecs   = 0;
        spi[i].speed_hz      = _speed;
        spi[i].bits_per_word = _bitsPerWord;
        spi[i].cs_change     = 0;
    }
    retval = ioctl (_spifd, SPI_IOC_MESSAGE(length), spi) ;
 
    if(retval < 0){
        perror("Problem transmitting spi data..ioctl");
        return -1;
    }
 
    return retval;
}
int mcp3008Spi::spi_Read(int32_t *buff, uint8_t channel)
{

    struct spi_ioc_transfer spi = {};
    int retval = -1;
    uint8_t tx[3] = {
        1,
        (uint8_t)(0x80 | ((channel & 7) << 4)),
        0
    };
    uint8_t rx[3] = {0};
    for (int i = 0; i < 3; i++)
    {
        spi.tx_buf        = (unsigned long)tx;
        spi.rx_buf        = (unsigned long)rx;
        spi.len           = 3; // <--- Send all 3 bytes continuously
        spi.delay_usecs   = 0;
        spi.speed_hz      = _speed;
        spi.bits_per_word = _bitsPerWord;
        spi.cs_change     = 0;
    }
    retval = ioctl (_spifd, SPI_IOC_MESSAGE(1), &spi);
    if(retval < 0){
        perror("Problem transmitting spi data..ioctl");
        return -1;
    }
    *buff = ((rx[1] & 0x03) << 8) | rx[2];
    return retval;
}

mcp3008Spi::~mcp3008Spi(){
    this->spiClose();
}