#include "RTE_Components.h"
#include CMSIS_device_header

#include "csp.h"
#include "csp_spi_conf.h"

namespace csp //------------------------------------------------------------------------------------
{

namespace spi //------------------------------------------------------------------------------------
{

res init(void)
{
    // GPIO
    GPIO_AF_INIT(SPI_PIN_MOSI, GPIO_AF5_SPI1, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
    GPIO_AF_INIT(SPI_PIN_MISO, GPIO_AF5_SPI1, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
    GPIO_AF_INIT(SPI_PIN_SCK , GPIO_AF5_SPI1, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
    GPIO_AF_INIT(SPI_PIN_NSS , GPIO_AF5_SPI1, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
    
    // SPI init MODE_MASTER, CPOL0, CPHA0, MSB_LSB, DATA_16_BITS, Max speed
    SPI_UNIT->CR1 = SPI_CR1_MSTR  | SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_DFF | SPI_CR1_SPE;
    
    return res::ok;
}

res send(uint8_t *const _buf, const uint32_t _len)
{
    return res::err;
}

__WEAK void cb_send(uint8_t *const _buf, const uint32_t _len)
{

}

res deinit(void)
{
    return res::err;
}

}; // namespace spi --------------------------------------------------------------------------------

}; // namespace csp --------------------------------------------------------------------------------