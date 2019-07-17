#pragma once

#include <hal.h>
#include <stdbool.h>
#include <stdint.h>

/* If the CPHA bit is set, the second edge on the SCK pin captures the first data bit transacted
   (falling edge if the CPOL bit is reset, rising edge if the CPOL bit is set). Data are latched on
   each occurrence of this clock transition type. If the CPHA bit is reset, the first edge on the
   SCK pin captures the first data bit transacted (falling edge if the CPOL bit is set, rising edge
   if the CPOL bit is reset). Data are latched on each occurrence of this clock transition type. */
#define SPI_DEVICE_FLAG_CPHA     (1<<0)
/* The CPOL (clock polarity) bit controls the idle state value of the clock when no data is being transferred.
   This bit affects both master and slave modes.
   If CPOL is reset, the SCK pin has a low-level idle state.
   If CPOL is set, the SCK pin has a high-level idle state. */
#define SPI_DEVICE_FLAG_CPOL     (1<<1)
#define SPI_DEVICE_FLAG_LSBFIRST (1<<2)
#define SPI_DEVICE_FLAG_SELPOL   (1<<3)

struct spi_device_s {
    uint32_t max_speed_hz;
    uint32_t sel_line;
    uint8_t bus_idx;
    uint8_t data_size;
    uint8_t flags;
    bool bus_acquired;

    SPIConfig spiconf;
};


bool spi_device_init(struct spi_device_s* dev, uint8_t bus_idx, uint32_t sel_line, uint32_t max_speed_hz, uint8_t data_size, uint8_t flags);
void spi_device_set_max_speed_hz(struct spi_device_s* dev, uint32_t max_speed_hz);
void spi_device_begin(struct spi_device_s* dev);
void spi_device_send(struct spi_device_s* dev, uint32_t n, const void* txbuf);
void spi_device_receive(struct spi_device_s* dev, uint32_t n, void* txbuf);
void spi_device_exchange(struct spi_device_s* dev, uint32_t n, const void* txbuf, void* rxbuf);
void spi_device_end(struct spi_device_s* dev);
