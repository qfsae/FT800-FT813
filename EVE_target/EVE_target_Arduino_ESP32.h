/*
@file    EVE_target_Arduino_ESP32.h
@brief   target specific includes, definitions and functions
@version 5.0
@date    2022-12-10
@author  Rudolph Riedel

@section LICENSE

MIT License

Copyright (c) 2016-2022 Rudolph Riedel

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

@section History

5.0
- extracted from EVE_target.h
- basic maintenance: checked for violations of white space and indent rules
- split up the optional default defines to allow to only change what needs changing thru the build-environment

*/


#ifndef EVE_TARGET_ARDUINO_ESP32_H
#define EVE_TARGET_ARDUINO_ESP32_H

#pragma once

#if defined(ARDUINO)

#include <stdint.h>
#include <Arduino.h>
#include "EVE_cpp_wrapper.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(ESP32)
/* note: this is using the ESP-IDF driver as the Arduino class and driver does not allow DMA for SPI */
#include "driver/spi_master.h"

/* you may define these in your build-environment to use different settings */
#if !defined(EVE_CS)
#define EVE_CS 13
#endif

#if !defined(EVE_PDN)
#define EVE_PDN 12
#endif

#if !defined(EVE_SCK)
#define EVE_SCK 18
#endif

#if !defined(EVE_MISO)
#define EVE_MISO 19
#endif

#if !defined(EVE_MOSI)
#define EVE_MOSI 23
#endif
/* you may define these in your build-environment to use different settings */


#define EVE_DMA

void EVE_init_spi(void);

extern spi_device_handle_t EVE_spi_device;
extern spi_device_handle_t EVE_spi_device_simple;

#if defined(EVE_DMA)
extern uint32_t EVE_dma_buffer[1025U];
extern volatile uint16_t EVE_dma_buffer_index;
extern volatile uint8_t EVE_dma_busy;

void EVE_init_dma(void);
void EVE_start_dma_transfer(void);
#endif

#define DELAY_MS(ms) delay(ms)

static inline void EVE_pdn_set(void)
{
    digitalWrite(EVE_PDN, LOW); /* go into power-down */
}

static inline void EVE_pdn_clear(void)
{
    digitalWrite(EVE_PDN, HIGH); /* power up */
}

static inline void EVE_cs_set(void)
{
    spi_device_acquire_bus(EVE_spi_device_simple, portMAX_DELAY);
    digitalWrite(EVE_CS, LOW); /* make EVE listen */
}

static inline void EVE_cs_clear(void)
{
    digitalWrite(EVE_CS, HIGH); /* tell EVE to stop listen */
    spi_device_release_bus(EVE_spi_device_simple);
}

static inline void spi_transmit(uint8_t data)
{
    spi_transaction_t trans = {0};
    trans.length = 8;
    trans.rxlength = 0;
    trans.flags = SPI_TRANS_USE_TXDATA;
    trans.tx_data[0] = data;
    spi_device_polling_transmit(EVE_spi_device_simple, &trans);
}

static inline void spi_transmit_32(uint32_t data)
{
    spi_transaction_t trans = {0};
    trans.length = 32;
    trans.rxlength = 0;
    trans.flags = 0;
    trans.tx_buffer = &data;
    spi_device_polling_transmit(EVE_spi_device_simple, &trans);
}

/* spi_transmit_burst() is only used for cmd-FIFO commands so it *always* has to transfer 4 bytes */
static inline void spi_transmit_burst(uint32_t data)
{
#if defined(EVE_DMA)
    EVE_dma_buffer[EVE_dma_buffer_index++] = data;
#else
    spi_transmit_32(data);
#endif
}

static inline uint8_t spi_receive(uint8_t data)
{
    spi_transaction_t trans = {0};
    trans.length = 8;
    trans.rxlength = 8;
    trans.flags = (SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA);
    trans.tx_data[0] = data;
    spi_device_polling_transmit(EVE_spi_device_simple, &trans);

    return trans.rx_data[0];
}

static inline uint8_t fetch_flash_byte(const uint8_t *data)
{
    return *data;
}

#endif /* ESP32 */

#ifdef __cplusplus
}
#endif

#endif /* Arduino */

#endif /* EVE_TARGET_ARDUINO_ESP32_H */
