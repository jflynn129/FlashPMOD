
#ifndef CUSTOM_BOARD_CONFIG_H
#define CUSTOM_BOARD_CONFIG_H

// PMOD board configuration is based off a nRF52840 Dev Board

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

// UART Interface Pins                                // Laird# -> Nordic#-> PMOD J1#
#define RX_PIN_NUMBER      NRF_GPIO_PIN_MAP (0,8)     // SIO_08 -> P0.08  -> J1.2
#define TX_PIN_NUMBER      NRF_GPIO_PIN_MAP (0,6)     // SIO_06 -> P0.06  -> J1.3
#define CTS_PIN_NUMBER     NRF_GPIO_PIN_MAP (0,7)     // SIO_07 -> P0.07  -> J1.4
#define RTS_PIN_NUMBER     NRF_GPIO_PIN_MAP (0,5)     // SIO_05 -> P0.05  -> J1.1

// Various GPIO functions
#define OTA_PIN_NUMBER     NRF_GPIO_PIN_MAP (0,2)     // SIO_02 -> P0.02  -> J1.7
#define RESET_PIN_NUMBER   NRF_GPIO_PIN_MAP (0,18)    // SIO_18 -> P0.18  -> J1.8
#define AUTORUN_PIN_NUMBER NRF_GPIO_PIN_MAP (1,3)     // SIO_35 -> P1.03  -> J1.9
#define WAKEUP_PIN_NUMBER  NRF_GPIO_PIN_MAP (0,31)    // SIO_31 -> P0.31  -> J1.10

// LED definitions for Avnet BLE PMOD peripheral board
// Each LED color is considered a separate LED

#define LED_0          NRF_GPIO_PIN_MAP(1,12)         // SIO_44 -> P1.12  (red)
#define LED_1          NRF_GPIO_PIN_MAP(1,11)         // SIO_43 -> P1.11  (green)
#define LED_2          NRF_GPIO_PIN_MAP(1,10)         // SIO_42 -> P1.10  (blue)

#define LEDS_NUMBER       4
#define LEDS_ACTIVE_STATE 0
#define LEDS_INV_MASK  LEDS_MASK
#define LEDS_LIST { LED_0, LED_0, LED_1, LED_2 }

// Macro to simply calling LEDs by colors

#define LED_RED        LED_0
#define LED_GREEN      LED_1
#define LED_BLUE       LED_2

#define LEDS_ACTIVE_STATE 0

// the SDK BSP files call for the following pin defines

#define BSP_LED_1             LED_0
#define BSP_LED_2             LED_1
#define BSP_LED_3             LED_2
#define BSP_SELF_PINRESET_PIN RESET_PIN_NUMBER
#define DFU_PIN_NUMBER        AUTORUN_PIN_NUMBER

#define HWFC           true

#define BSP_BUTTON_0 9

#ifdef __cplusplus
}
#endif

#endif // CUSTOM_BOARD_CONFIG_H
