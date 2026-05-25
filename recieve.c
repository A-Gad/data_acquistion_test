#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

static const char *device = "/dev/spidev1.1";
static uint32_t mode;
static uint8_t bits = 8;
static char *input_file;
static char *output_file;
static uint32_t speed = 500000;
static uint16_t delay;
static uint16_t word_delay;
static int verbose;
static int transfer_size;
static int iterations;
static int interval = 5;

static void pabort(const char *s)
{
	if (errno != 0)
		perror(s);
	else
		printf("%s\n", s);

	abort();
}

int32_t mcp3008_read(int fd) 
{
    
    int ret;
    const int32_t tx = 0b000000011000;
	
    const int32_t rx = 0;
	int len = sizeof(tx);

	
    struct spi_ioc_transfer tr = {
		.tx_buf = tx,
		.rx_buf = rx,
		.len = len,
        .speed_hz = speed,
        .bits_per_word = bits,
        .cs_change = false,
		.word_delay_usecs = word_delay,
	};
	uint8_t mode = SPI_LOOP;

	ret = ioctl(fd, SPI_IOC_WR_MODE,&mode);
	if (ret = -1)
		pabort("cannot set mode");	
	ret = ioctl(fd, SPI_IOC_RD_MODE,&mode);
	if (ret = -1)
		pabort("cannot set mode");	
	/*
	 *bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: 0x%x\n", mode);
	printf("bits per word: %u\n", bits);
	printf("max speed: %u Hz (%u kHz)\n", speed, speed/1000);


    ret = ioctl(fd,SPI_IOC_MESSAGE(1),&tr);
    if (ret < 1)
		pabort("can't send spi message");

    return rx;
}
