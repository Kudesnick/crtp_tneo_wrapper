#pragma once

#include "RTE_Components.h"
#include CMSIS_device_header
#include "stm32f4xx_hal_gpio.h"

#include <stdint.h>

namespace csp
{

namespace gpio
{

enum class port
{
    na = 0xFF,

    a00 = 0x00,
    a01 = 0x01,
    a02 = 0x02,
    a03 = 0x03,
    a04 = 0x04,
    a05 = 0x05,
    a06 = 0x06,
    a07 = 0x07,
    a08 = 0x08,
    a09 = 0x09,
    a10 = 0x0A,
    a11 = 0x0B,
    a12 = 0x0C,
    a13 = 0x0D,
    a14 = 0x0E,
    a15 = 0x0F,

    b00 = 0x10,
    b01 = 0x11,
    b02 = 0x12,
    b03 = 0x13,
    b04 = 0x14,
    b05 = 0x15,
    b06 = 0x16,
    b07 = 0x17,
    b08 = 0x18,
    b09 = 0x19,
    b10 = 0x1A,
    b11 = 0x1B,
    b12 = 0x1C,
    b13 = 0x1D,
    b14 = 0x1E,
    b15 = 0x1F,

    c00 = 0x20,
    c01 = 0x21,
    c02 = 0x22,
    c03 = 0x23,
    c04 = 0x24,
    c05 = 0x25,
    c06 = 0x26,
    c07 = 0x27,
    c08 = 0x28,
    c09 = 0x29,
    c10 = 0x2A,
    c11 = 0x2B,
    c12 = 0x2C,
    c13 = 0x2D,
    c14 = 0x2E,
    c15 = 0x2F,

    d00 = 0x30,
    d01 = 0x31,
    d02 = 0x32,
    d03 = 0x33,
    d04 = 0x34,
    d05 = 0x35,
    d06 = 0x36,
    d07 = 0x37,
    d08 = 0x38,
    d09 = 0x39,
    d10 = 0x3A,
    d11 = 0x3B,
    d12 = 0x3C,
    d13 = 0x3D,
    d14 = 0x3E,
    d15 = 0x3F,

    e00 = 0x40,
    e01 = 0x41,
    e02 = 0x42,
    e03 = 0x43,
    e04 = 0x44,
    e05 = 0x45,
    e06 = 0x46,
    e07 = 0x47,
    e08 = 0x48,
    e09 = 0x49,
    e10 = 0x4A,
    e11 = 0x4B,
    e12 = 0x4C,
    e13 = 0x4D,
    e14 = 0x4E,
    e15 = 0x4F,

    f00 = 0x50,
    f01 = 0x51,
    f02 = 0x52,
    f03 = 0x53,
    f04 = 0x54,
    f05 = 0x55,
    f06 = 0x56,
    f07 = 0x57,
    f08 = 0x58,
    f09 = 0x59,
    f10 = 0x5A,
    f11 = 0x5B,
    f12 = 0x5C,
    f13 = 0x5D,
    f14 = 0x5E,
    f15 = 0x5F,

    g00 = 0x60,
    g01 = 0x61,
    g02 = 0x62,
    g03 = 0x63,
    g04 = 0x64,
    g05 = 0x65,
    g06 = 0x66,
    g07 = 0x67,
    g08 = 0x68,
    g09 = 0x69,
    g10 = 0x6A,
    g11 = 0x6B,
    g12 = 0x6C,
    g13 = 0x6D,
    g14 = 0x6E,
    g15 = 0x6F,
};


inline uint8_t        port_source(const port _GPIO) {return static_cast<int>(_GPIO) >> 4;}
inline uint8_t        pin_source(const port _GPIO)  {return static_cast<int>(_GPIO) & 0xF;}
inline GPIO_TypeDef * port_addr(const port _GPIO)   {return ((GPIO_TypeDef *)(GPIOA_BASE + 0x400 * port_source(_GPIO)));}
inline uint32_t       pin(const port _GPIO)         {return 1U << pin_source(_GPIO);}
inline uint32_t       rcc(const port _GPIO)         {return 1U << port_source(_GPIO);}
inline uint8_t        adc_channel(const port _GPIO) {return pin_source(_GPIO);}
inline uint8_t        exti_line(const port _GPIO)   {return pin(_GPIO);}


inline constexpr uint8_t irq_channel(const port _GPIO)
{
    const uint8_t res[]
    {
        EXTI0_IRQn,    
        EXTI1_IRQn,    
        EXTI2_IRQn,    
        EXTI3_IRQn,
        EXTI4_IRQn,
        EXTI9_5_IRQn,
        EXTI9_5_IRQn,
        EXTI9_5_IRQn,
        EXTI9_5_IRQn,
        EXTI9_5_IRQn,
        EXTI15_10_IRQn,
        EXTI15_10_IRQn,
        EXTI15_10_IRQn,
        EXTI15_10_IRQn,
        EXTI15_10_IRQn,
        EXTI15_10_IRQn,
    };

    return res[pin_source(_GPIO)];
}

inline void init(const port _GPIO, const uint32_t _AF_NUM, const uint32_t _AF_MODE, const uint32_t _PU, const uint32_t _SPEED)
{
    RCC->AHB1ENR |= rcc(_GPIO);
    auto port_ptr = port_addr(_GPIO);
    auto pin_src =  pin_source(_GPIO);
    
    port_ptr->AFR[pin_src >> 3] |= (_AF_NUM) << ((pin_src & 7) * 4);
    port_ptr->MODER   &= ~(3U << (pin_src * 2));
    port_ptr->MODER   |= (_AF_MODE & 3) << (pin_src * 2);
    port_ptr->OTYPER  |= (_AF_MODE >> 4) << pin_src;
    port_ptr->OSPEEDR |= _SPEED << (pin_src * 2);
    port_ptr->PUPDR   &= ~(3U << (pin_src * 2));
    port_ptr->PUPDR   |= _PU << (pin_src * 2);
}

inline void set(const port _GPIO, const bool _STATE)
{
    port_addr(_GPIO)->BSRR = pin(_GPIO) << (16 - _STATE * 16);
}

inline void toggle(const port _GPIO)
{
    auto port_ptr = port_addr(_GPIO);
    auto io_pin = pin(_GPIO);
    const uint32_t odr = port_ptr->ODR;
    port_ptr->BSRR = ((odr & io_pin) << 16) | (~odr & io_pin);
}

}; // namespace gpio

}; // namespace csp
