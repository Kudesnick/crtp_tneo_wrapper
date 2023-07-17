#pragma once

#include "csp_gpio_macro.h"

#define SPI_UNIT            SPI1
    
#define SPI_PIN_MOSI        port::a07
#define SPI_PIN_MISO        port::a06
#define SPI_PIN_SCK         port::a05
#define SPI_PIN_NSS         port::a04

#define SPI_DMA_RX          DMA2_Stream2
#define SPI_DMA_RX_CH       (3U)
#define SPI_DMA_TX          DMA2_Stream3
#define SPI_DMA_TX_CH       (3U)
