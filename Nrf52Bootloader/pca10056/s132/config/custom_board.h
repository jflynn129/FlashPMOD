/**
 * Copyright (c) 2017 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef CUSTOM_BOARD_CONFIG_H
#define CUSTOM_BOARD_CONFIG_H

// our custom board configuration is based off this board... 
// #undef  BOARD_PCA10056  // nRF52840 Dev Board
// #include "pca10056.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

// LED definitions for Avnet BLE PMOD peripheral board
// Each LED color is considered a separate LED
#define LEDS_NUMBER    3

#undef RX_PIN_NUMBER  
#undef TX_PIN_NUMBER  
#undef CTS_PIN_NUMBER
#undef RTS_PIN_NUMBER

#define RX_PIN_NUMBER  NRF_GPIO_PIN_MAP (0,08)    // P0.08
#define TX_PIN_NUMBER  NRF_GPIO_PIN_MAP (0,06)    // P0.06  
#define CTS_PIN_NUMBER NRF_GPIO_PIN_MAP (0,07)    // P0.07
#define RTS_PIN_NUMBER NRF_GPIO_PIN_MAP (0,05)    // P0.05
// #define RESET_PIN_NUMBER NRF_GPIO_PIN_MAP (0,18)  // P0.18
// #define DFU_PIN_NUMBER NRF_GPIO_PIN_MAP (1,03)    // P0.35

// #define LED1_G         NRF_GPIO_PIN_MAP(0,6)
#define LED2_R         NRF_GPIO_PIN_MAP(1,12)     // P1.12 / SIO_44
#define LED2_G         NRF_GPIO_PIN_MAP(1,11)     // P1.11 / SIO_43
#define LED2_B         NRF_GPIO_PIN_MAP(1,10)     // P1.10 / SIO_42

// #define LED_1          LED1_G
#define LED_2          LED2_R
#define LED_3          LED2_G
#define LED_4          LED2_B

#define LEDS_ACTIVE_STATE 0

#define LEDS_LIST { LED_2, LED_3, LED_4 }

#define LEDS_INV_MASK  LEDS_MASK

// #define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2
#define BSP_LED_2      LED_3
#define BSP_LED_3      LED_4

// There are NO buttons for the application
// as the second button is used for a RESET.
// #define BUTTONS_NUMBER 0

// #define BUTTON_1       NRF_GPIO_PIN_MAP(1,6)
// #define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

// #define BUTTONS_ACTIVE_STATE 0

// #define BUTTONS_LIST { BUTTON_1 }

// #define BSP_BUTTON_0   BUTTON_1

//pf
// #define BSP_SELF_PINRESET_PIN NRF_GPIO_PIN_MAP(0,18)

#define HWFC           true

#ifdef __cplusplus
}
#endif

#endif // CUSTOM_BOARD_CONFIG_H
