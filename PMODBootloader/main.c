/**
 * Copyright (c) 2016 - 2018, Nordic Semiconductor ASA
 * All rights reserved.
 *
 */
/** @file
 *
 * @defgroup bootloader_secure_ble main.c
 * @{
 * @ingroup dfu_bootloader_api
 * @brief Bootloader project main file for secure DFU.
 *
 */

#include <stdint.h>
#include "boards.h"
#include "nrf_mbr.h"
#include "nrf_bootloader.h"
#include "nrf_bootloader_app_start.h"
#include "nrf_bootloader_dfu_timers.h"
#include "nrf_dfu.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_error.h"
#include "app_error_weak.h"
#include "nrf_bootloader_info.h"
#include "nrf_delay.h"

//PMOD LED Colors
#define BLACK_LED      0  //0000 BLACK (off) 
#define RED_LED        1  //0001 RED
#define GREEN_LED      2  //0010 GREEN
#define YELLOW_LED     3  //0011 RED + GREEN
#define BLUE_LED       4  //0100
#define MAGENTA_LED    5  //0101 BLUE + RED
#define CYAN_LED       6  //0110 GREEN+ BLUE
#define WHITE_LED      7  //0111 GREEN+BLUE+RED


void set_color( uint16_t color )
{
    switch( color ) {
        case BLACK_LED:
            bsp_board_led_off(BSP_BOARD_LED_2);
            bsp_board_led_off(BSP_BOARD_LED_3);
            bsp_board_led_off(BSP_BOARD_LED_1);
            break;
        case RED_LED:
            bsp_board_led_off(BSP_BOARD_LED_2);
            bsp_board_led_off(BSP_BOARD_LED_3);
            bsp_board_led_on(BSP_BOARD_LED_1);
            break;
        case GREEN_LED:
            bsp_board_led_on(BSP_BOARD_LED_2);
            bsp_board_led_off(BSP_BOARD_LED_3);
            bsp_board_led_off(BSP_BOARD_LED_1);
            break;
        case YELLOW_LED:
            bsp_board_led_on(BSP_BOARD_LED_2);
            bsp_board_led_off(BSP_BOARD_LED_3);
            bsp_board_led_on(BSP_BOARD_LED_1);
            break;
        case BLUE_LED:
            bsp_board_led_off(BSP_BOARD_LED_2);
            bsp_board_led_on(BSP_BOARD_LED_3);
            bsp_board_led_off(BSP_BOARD_LED_1);
            break;
        case MAGENTA_LED:
            bsp_board_led_off(BSP_BOARD_LED_2);
            bsp_board_led_on(BSP_BOARD_LED_3);
            bsp_board_led_on(BSP_BOARD_LED_1);
            break;
        case CYAN_LED:
            bsp_board_led_on(BSP_BOARD_LED_2);
            bsp_board_led_on(BSP_BOARD_LED_3);
            bsp_board_led_off(BSP_BOARD_LED_1);
            break;
        case WHITE_LED:
            bsp_board_led_on(BSP_BOARD_LED_2);
            bsp_board_led_on(BSP_BOARD_LED_3);
            bsp_board_led_on(BSP_BOARD_LED_1);
            break;
        }
}


static void on_error(void)
{
    NRF_LOG_FINAL_FLUSH();

#if NRF_MODULE_ENABLED(NRF_LOG_BACKEND_RTT)
    // To allow the buffer to be flushed by the host.
    nrf_delay_ms(100);
#endif
#ifdef NRF_DFU_DEBUG_VERSION
    NRF_BREAKPOINT_COND;
#endif
    NVIC_SystemReset();
}


void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    NRF_LOG_ERROR("%s:%d", p_file_name, line_num);
    on_error();
}


void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    NRF_LOG_ERROR("Received a fault! id: 0x%08x, pc: 0x%08x, info: 0x%08x", id, pc, info);
    on_error();
}


void app_error_handler_bare(uint32_t error_code)
{
    NRF_LOG_ERROR("Received an error: 0x%08x!", error_code);
    on_error();
}

/**
 * @brief Function notifies certain events in DFU process.
 */
static void dfu_observer(nrf_dfu_evt_type_t evt_type)
{
    switch (evt_type)
    {
        case NRF_DFU_EVT_DFU_FAILED:
        case NRF_DFU_EVT_DFU_ABORTED:
        case NRF_DFU_EVT_DFU_INITIALIZED:
        //jmf
        //initialize LEDs on PMOD
            bsp_board_init(BSP_INIT_LEDS);
            set_color(RED_LED);
            break;
        case NRF_DFU_EVT_TRANSPORT_ACTIVATED:
            set_color(GREEN_LED);
            break;
        case NRF_DFU_EVT_DFU_STARTED:
            break;
        default:
            break;
    }
}


/**@brief Function for application main entry. */
int main(void)
{
    uint32_t ret_val;

    // Protect MBR and bootloader code from being overwritten.
    ret_val = nrf_bootloader_flash_protect(0, MBR_SIZE, false);
    APP_ERROR_CHECK(ret_val);
    ret_val = nrf_bootloader_flash_protect(BOOTLOADER_START_ADDR, BOOTLOADER_SIZE, false);
    APP_ERROR_CHECK(ret_val);

    (void) NRF_LOG_INIT(nrf_bootloader_dfu_timer_counter_get);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_INFO("Inside main");

    ret_val = nrf_bootloader_init(dfu_observer);
    APP_ERROR_CHECK(ret_val);

    // Either there was no DFU functionality enabled in this project or the DFU module detected
    // no ongoing DFU operation and found a valid main application.
    // Boot the main application.
    nrf_bootloader_app_start();

    // Should never be reached.
    NRF_LOG_INFO("After main");
}

/**
 * @}
 */
