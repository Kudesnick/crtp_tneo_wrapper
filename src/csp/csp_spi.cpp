#include "RTE_Components.h"
#include CMSIS_device_header

#include "csp.h"
#include "csp_spi_conf.h"

namespace csp //------------------------------------------------------------------------------------
{

constexpr static inline uint8_t mask_to_bit(const uint32_t _mask)
{
    return (_mask & 1 <<  0) ?  0 :
           (_mask & 1 <<  1) ?  1 :
           (_mask & 1 <<  2) ?  2 :
           (_mask & 1 <<  3) ?  3 :
           (_mask & 1 <<  4) ?  4 :
           (_mask & 1 <<  5) ?  5 :
           (_mask & 1 <<  6) ?  6 :
           (_mask & 1 <<  7) ?  7 :
           (_mask & 1 <<  8) ?  8 :
           (_mask & 1 <<  9) ?  9 :
           (_mask & 1 << 10) ? 10 :
           (_mask & 1 << 11) ? 11 :
           (_mask & 1 << 12) ? 12 :
           (_mask & 1 << 13) ? 13 :
           (_mask & 1 << 14) ? 14 :
           (_mask & 1 << 15) ? 15 :
           (_mask & 1 << 16) ? 16 :
           (_mask & 1 << 17) ? 17 :
           (_mask & 1 << 18) ? 18 :
           (_mask & 1 << 19) ? 19 :
           (_mask & 1 << 20) ? 20 :
           (_mask & 1 << 21) ? 21 :
           (_mask & 1 << 22) ? 22 :
           (_mask & 1 << 23) ? 23 :
           (_mask & 1 << 24) ? 24 :
           (_mask & 1 << 25) ? 25 :
           (_mask & 1 << 26) ? 26 :
           (_mask & 1 << 27) ? 27 :
           (_mask & 1 << 28) ? 28 :
           (_mask & 1 << 29) ? 29 :
           (_mask & 1 << 30) ? 30 :
           (_mask & 1 << 31) ? 31 : 0;
}

constexpr static inline uint32_t addr_calc(const uint32_t _addr, const uint8_t _bit)
{
    return (_addr & 0xF0000000) + ((_addr & 0x00FFFFFF) << 5) + 0x02000000 + (_bit << 2);
}
  
static inline uint32_t & bb(volatile uint32_t & _addr, const uint32_t _mask)
{
    return *reinterpret_cast<uint32_t *const>(addr_calc(reinterpret_cast<const uint32_t>(&_addr), mask_to_bit(_mask)));
}


namespace spi //------------------------------------------------------------------------------------
{

static uint8_t *dest;
static uint32_t len;

static inline void irq_dma_reset(const uint8_t _strn)
{
    // Clear all interrupt flags
    ((_strn <= 3) ? SPI_DMA->LIFCR : SPI_DMA->HIFCR) = ((1UL << 6) - 1) << (6 * (_strn & 1)) << (16 * ((_strn & 2) >> 1));
}

static inline void irq_rx_reset(void)
{
    // Clear all interrupt flags
    ((SPI_DMA_RX_STRn <= 3) ? SPI_DMA->LIFCR : SPI_DMA->HIFCR) = ((1UL << 6) - 1) << (6 * (SPI_DMA_RX_STRn & 1)) << (16 * ((SPI_DMA_RX_STRn & 2) >> 1));
}

static void dma_start(const uint32_t _len)
{
    SPI_DMA_RX->NDTR  = _len;
    SPI_DMA_TX->NDTR  = _len;

    irq_rx_reset();
    __NVIC_EnableIRQ(SPI_DMA_RX_IRQn);
    SPI_DMA_RX->CR   |= DMA_SxCR_EN; // Enable DMA

    irq_dma_reset(SPI_DMA_TX_STRn);
    __NVIC_EnableIRQ(SPI_DMA_TX_IRQn);
    SPI_DMA_TX->CR   |= DMA_SxCR_EN; // Enable DMA
}

res init(void)
{
    // GPIO init
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    RCC->AHB1ENR |= (SPI_DMA == DMA1) ? RCC_AHB1ENR_DMA1EN : RCC_AHB1ENR_DMA2EN;
    // GPIO
    gpio::init(SPI_PIN_MOSI, GPIO_AF5_SPI1, GPIO_MODE_AF_PP    , GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
    gpio::init(SPI_PIN_MISO, GPIO_AF5_SPI1, MODE_AF            , GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
    gpio::init(SPI_PIN_SCK , GPIO_AF5_SPI1, GPIO_MODE_AF_PP    , GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
    gpio::init(SPI_PIN_NSS , 0            , GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);

    gpio::set(SPI_PIN_NSS, true);

    // Reset unit
    SPI_UNIT->CR1 &= ~SPI_CR1_SPE;
    while(SPI_UNIT->CR1 &= ~SPI_CR1_SPE);
    // SPI init MODE_MASTER, CPOL0, CPHA0, MSB_LSB, DATA_8_BITS, Max speed
    SPI_UNIT->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
#ifdef DEBUG
    SPI_UNIT->CR1 |= 7 << SPI_CR1_BR_Pos;
#endif

    // Rx DMA init
    SPI_DMA_RX->CR   &= ~DMA_SxCR_EN; // Disable DMA
    while(SPI_DMA_RX->CR & DMA_SxCR_EN);
    SPI_DMA_RX->CR    = (SPI_DMA_RX_CH << DMA_SxCR_CHSEL_Pos) | DMA_SxCR_MINC
                        | (3 << DMA_SxCR_PL_Pos)
                        /* | DMA_SxCR_DIR_1 | DMA_SxCR_DIR_0 */; // Peripheral-to-memory
    SPI_DMA_RX->FCR   = 0;
    SPI_DMA_RX->PAR   = (uint32_t)&(SPI_UNIT->DR);
    SPI_DMA_RX->CR   |= DMA_SxCR_TCIE;
    SPI_UNIT->CR2    |= SPI_CR2_RXDMAEN;

    // Tx DMA init
    SPI_DMA_TX->CR   &= ~DMA_SxCR_EN; // Disable DMA
    while(SPI_DMA_TX->CR & DMA_SxCR_EN);
    SPI_DMA_TX->CR    = (SPI_DMA_TX_CH << DMA_SxCR_CHSEL_Pos) | DMA_SxCR_MINC
                        /* | DMA_SxCR_DIR_1 */ | DMA_SxCR_DIR_0; // Memory-to-peripheral
    SPI_DMA_TX->FCR   = 0;
    SPI_DMA_TX->PAR   = (uint32_t)&(SPI_UNIT->DR);
    SPI_DMA_TX->CR   |= DMA_SxCR_TCIE;
    SPI_UNIT->CR2    |= SPI_CR2_TXDMAEN;

    bb(SPI_UNIT->CR1, SPI_CR1_SPE) = 1;

    return res::ok;
}


void cs_on(void) {gpio::set(SPI_PIN_NSS, false);};
void cs_off(void) {gpio::set(SPI_PIN_NSS, true);};


res deinit(void)
{
    return res::err;
}


res send(const uint8_t _source[], const uint32_t _len, uint8_t *const _dest)
{
    if (SPI1->SR & SPI_SR_BSY) return res::err;

    dest = _dest;
    len = 0;
    
    if (dest != nullptr)
    {
        len = _len;
        SPI_DMA_RX->M0AR  = reinterpret_cast<uint32_t>(_dest);
        SPI_DMA_RX->CR   |= DMA_SxCR_MINC;        
    }
    else
    {
        static uint8_t dummy;
        SPI_DMA_RX->M0AR  = reinterpret_cast<uint32_t>(&dummy);
        SPI_DMA_RX->CR   &= ~DMA_SxCR_MINC;
    }

    SPI_DMA_TX->M0AR  = reinterpret_cast<uint32_t>(_source);
    SPI_DMA_TX->CR   |= DMA_SxCR_MINC;

    dma_start(_len);
    return res::ok;
}

res read(const uint8_t _dummy, uint8_t _dest[], const uint32_t _len)
{
    if (SPI1->SR & SPI_SR_BSY) return res::err;

    static const uint8_t dummy = _dummy;

    dest = _dest;
    len = _len;
    
    SPI_DMA_RX->M0AR  = reinterpret_cast<uint32_t>(_dest);
    SPI_DMA_RX->CR   |= DMA_SxCR_MINC;

    SPI_DMA_TX->M0AR  = reinterpret_cast<uint32_t>(&dummy);
    SPI_DMA_TX->CR   &= ~DMA_SxCR_MINC;

    dma_start(_len);
    return res::ok;
}

__WEAK void cb_send(uint8_t *const _dest, const uint32_t _dest_len)
{
    (void)_dest;
    (void)_dest_len;
}

}; // namespace spi --------------------------------------------------------------------------------

}; // namespace csp --------------------------------------------------------------------------------

// interrupt vectros -------------------------------------------------------------------------------

extern "C" void SPI_DMA_RX_IRQHandler(void);
void SPI_DMA_RX_IRQHandler(void)
{
    csp::spi::irq_rx_reset();
    csp::spi::cb_send(csp::spi::dest, csp::spi::len);
}

extern "C" void SPI_DMA_TX_IRQHandler(void);
void SPI_DMA_TX_IRQHandler(void)
{
    csp::spi::irq_dma_reset(SPI_DMA_TX_STRn);
}
