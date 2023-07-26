#pragma once

#include "csp_gpio_macro.h"

#define SPI_UNIT              SPI1
    
#define SPI_PIN_MOSI          port::a07
#define SPI_PIN_MISO          port::a06 // V2.0 MISO脚由PA6改为PB4,方便STM32F411用户同时连接TF卡和FLASH
#define SPI_PIN_SCK           port::a05
#define SPI_PIN_NSS           port::a04

#define SPI_DMA               DMA2

#define SPI_DMA_RX            DMA2_Stream0
#define SPI_DMA_RX_STRn       (0U)
#define SPI_DMA_RX_IRQn       DMA2_Stream0_IRQn
#define SPI_DMA_RX_IRQHandler DMA2_Stream0_IRQHandler
#define SPI_DMA_RX_CH         (3U)

#define SPI_DMA_TX            DMA2_Stream2
#define SPI_DMA_TX_STRn       (2U)
#define SPI_DMA_TX_IRQn       DMA2_Stream2_IRQn
#define SPI_DMA_TX_IRQHandler DMA2_Stream2_IRQHandler
#define SPI_DMA_TX_CH         (2U)
