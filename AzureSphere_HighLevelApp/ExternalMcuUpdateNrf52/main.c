/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// applibs_versions.h defines the API struct versions to use for applibs APIs.
#include "applibs_versions.h"
#include "epoll_timerfd_utilities.h"
#include <applibs/uart.h>
#include <applibs/gpio.h>
#include <applibs/log.h>

// By default, this sample is targeted at the MT3620 Reference Development Board (RDB).
// This can be changed using the project property "Target Hardware Definition Directory".
// This #include imports the sample_hardware abstraction from that hardware definition.
#include <hw/sample_hardware.h>

#include "nordic/dfu_uart_protocol.h"

// The file descriptors are initialized to an invalid value so they can
// be cleaned up safely if they are only partially initialized.
static int epollFd = -1;
static int nrfUartFd = -1;
static int nrfResetGpioFd = -1;
static int nrfDfuModeGpioFd = -1;
static int triggerUpdateButtonGpioFd = -1;
static int triggerResetButtonGpioFd = -1;
static int buttonPollTimerFd = -1;

// State variables
static GPIO_Value_Type buttonState = GPIO_Value_High;
static GPIO_Value_Type BbuttonState = GPIO_Value_High;

// To write an image to the Nordic board, add the data and binary files as
// resources to the solution and modify this object. The first image should
// be the softdevice; the second image is the application.
static DfuImageData images[] = { 
		                        {.datPathname = "s132_nrf52_6.1.0_softdevice.dat",
								 .binPathname = "s132_nrf52_6.1.0_softdevice.bin",
								 .firmwareType = DfuFirmware_Softdevice,
								 .version = 6001000
	                             },
                                {.datPathname = "blinkyV1.dat",
                                 .binPathname = "blinkyV1.bin",
                                 .firmwareType = DfuFirmware_Application,
                                 .version = 1
                                 }
                                };

static const size_t imageCount = sizeof(images) / sizeof(images[0]);

// Whether currently writing images to attached board.
static bool inDfuMode = false;

// Termination state
static volatile sig_atomic_t terminationRequired = false;

/// <summary>
///     Signal handler for termination requests. This handler must be async-signal-safe.
/// </summary>
static void TerminationHandler(int signalNumber)
{
    // Don't use Log_Debug here, as it is not guaranteed to be async-signal-safe.
    terminationRequired = true;
}

void DfuTerminationHandler(DfuResultStatus status)
{
    Log_Debug("\nFinished updating images with status: %s, setting DFU mode to false.\n",
              status == DfuResult_Success ? "SUCCESS" : "FAILED");
    inDfuMode = false;
}

/// <summary>
///     Handle button timer event: if the button is pressed, trigger DFU mode and send updates.
/// </summary>
static void ButtonPollTimerEventHandler(EventData *eventData)
{
    if (ConsumeTimerFdEvent(buttonPollTimerFd) != 0) {
        terminationRequired = true;
        return;
    }
    // Check for a button press
    GPIO_Value_Type newButtonState;
    int result = GPIO_GetValue(triggerUpdateButtonGpioFd, &newButtonState);
    if (result != 0) {
        Log_Debug("ERROR: Could not read button GPIO: %s (%d).\n", strerror(errno), errno);
        terminationRequired = true;
        return;
    }

    // If the button has just been pressed and we are not in DFU mode, we trigger the update.
    // The button has GPIO_Value_Low when pressed and GPIO_Value_High when released.
    if (newButtonState != buttonState) {
        if (newButtonState == GPIO_Value_Low) {
            if (!inDfuMode) {
                Log_Debug("\nStarting firmware update...\n");
                inDfuMode = true;
                ProgramImages(images, imageCount, &DfuTerminationHandler);
            }
        }
        buttonState = newButtonState;
    }

	// check to see if the user wants to reset the PMOD and start preprogrammed operations
	result = GPIO_GetValue(triggerResetButtonGpioFd, &newButtonState);
	if (result != 0) {
		Log_Debug("ERROR: Could not read SAMPLE_BUTTON_2 GPIO: %s (%d).\n", strerror(errno), errno);
		terminationRequired = true;
		return;
	}
	if (BbuttonState != newButtonState) {
		if (newButtonState == GPIO_Value_Low) {
			GPIO_SetValue(nrfResetGpioFd, GPIO_Value_Low);
			Log_Debug("\nAssert Reset\n");
			inDfuMode = false;
			BbuttonState = newButtonState;
		}
		else {
			GPIO_SetValue(nrfResetGpioFd, GPIO_Value_High);
			Log_Debug("\nRestart PMOD\n");
		}
		BbuttonState = newButtonState;
	}
}

// event handler data structures. Only the event handler field needs to be populated.
static EventData buttonPollTimerEvent = {.eventHandler = &ButtonPollTimerEventHandler};

/// <summary>
///     Set up SIGTERM termination handler, initialize peripherals, and set up event handlers.
/// </summary>
/// <returns>0 on success, or -1 on failure</returns>
/// <notes>
///     This Demonstratioin Program uses the following IO interfaces:
///      - SAMPLE_NRF52_RESET (GPIO28)
///      - SAMPLE_NRF52_DFU   (GPIO26)
///      - SAMPLE_BUTTON_1    (BUTTON A)
///      - SAMPLE_BUTTON_2    (BUTTON B)
///      - SAMPLE_NRF52_UART  (ISU1)
static int InitPeripheralsAndHandlers(void)
{
	nrfResetGpioFd = GPIO_OpenAsOutput(SAMPLE_NRF52_RESET, GPIO_OutputMode_OpenDrain, GPIO_Value_High);
    if (nrfResetGpioFd == -1) {
        Log_Debug("ERROR: Could not open SAMPLE_NRF52_RESET: %s (%d).\n", strerror(errno), errno);
        return -1;
    }
    Log_Debug("\n\nAssert SAMPLE_NRF52_RESET\n");
    GPIO_SetValue(nrfResetGpioFd, GPIO_Value_Low);

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

    epollFd = CreateEpollFd();
    if (epollFd == -1) {
        return -1;
    }

    // Create a UART_Config object, open the UART and set up UART event handler
    UART_Config uartConfig;
    UART_InitConfig(&uartConfig);
    uartConfig.baudRate = 115200;
    uartConfig.flowControl = UART_FlowControl_RTSCTS;
    nrfUartFd = UART_Open(SAMPLE_NRF52_UART, &uartConfig);
    if (nrfUartFd == -1) {
        Log_Debug("ERROR: Could not open UART: %s (%d).\n", strerror(errno), errno);
        return -1;
    }
    Log_Debug("Open UART for communicatioins. .\n");
    // uartFd will be added to the epoll when needed (check epoll protocol)

    nrfDfuModeGpioFd = GPIO_OpenAsOutput(SAMPLE_NRF52_DFU, GPIO_OutputMode_OpenDrain, GPIO_Value_High);
    if (nrfDfuModeGpioFd == -1) {
        Log_Debug("ERROR: Could not open SAMPLE_NRF52_DFU: %s (%d).\n", strerror(errno), errno);
        return -1;
    }
    Log_Debug("Opening SAMPLE_NRF52_DFU to notify PMOD when to download the firmware\n");

    InitUartProtocol(nrfUartFd, nrfResetGpioFd, nrfDfuModeGpioFd, epollFd);

    Log_Debug("Opening SAMPLE_BUTTON_1 (Button A) as input\n");
    triggerUpdateButtonGpioFd = GPIO_OpenAsInput(SAMPLE_BUTTON_1);
    if (triggerUpdateButtonGpioFd == -1) {
        Log_Debug("ERROR: Could not open button GPIO: %s (%d).\n", strerror(errno), errno);
        return -1;
    }

	Log_Debug("Opening SAMPLE_BUTTON_2 (Button B) as input\n");
	triggerResetButtonGpioFd = GPIO_OpenAsInput(SAMPLE_BUTTON_2);
	if (triggerResetButtonGpioFd == -1) {
		Log_Debug("ERROR: Could not open button GPIO: %s (%d).\n", strerror(errno), errno);
		return -1;
	}

    struct timespec buttonPressCheckPeriod = {0, 1000000};
    buttonPollTimerFd = CreateTimerFdAndAddToEpoll(epollFd, &buttonPressCheckPeriod, &buttonPollTimerEvent, EPOLLIN);
    if (buttonPollTimerFd == -1) {
        return -1;
    }

    inDfuMode = false;  //not in DFU mode until the user presses SAMPLE_BUTTON_1

    // Take nRF52 out of reset, allowing its application to start
    Log_Debug("Deassert SAMPLE_NRF52_RESET to start PMOD operating\n\n");
    GPIO_SetValue(nrfResetGpioFd, GPIO_Value_High);

    return 0;
}

/// <summary>
///     Close peripherals and handlers.
/// </summary>
static void ClosePeripheralsAndHandlers(void)
{
    Log_Debug("Closing all file descriptors\n");
    CloseFdAndPrintError(buttonPollTimerFd, "ButtonPollTimer");
    CloseFdAndPrintError(triggerUpdateButtonGpioFd, "TriggerUpdateButtonGpio");
	CloseFdAndPrintError(triggerResetButtonGpioFd, "TriggerResetButtonGpio");
    CloseFdAndPrintError(nrfResetGpioFd, "NrfResetGpio");
    CloseFdAndPrintError(nrfDfuModeGpioFd, "NrfDfuModeGpio");
    CloseFdAndPrintError(nrfUartFd, "NrfUart");
    CloseFdAndPrintError(epollFd, "Epoll");
}

/// <summary>
///     Main entry point for this application.
/// </summary>
int main(int argc, char *argv[])
{
	Log_Debug("\n");
	Log_Debug("     ****\r\n");
	Log_Debug("    **  **     PMOD/DFU Firmware Update Demonstration\\r\n");
	Log_Debug("   **    **    for the Avnet\r\n");
	Log_Debug("  ** ==== **   Azure Sphere MT3620 Starter Kit\r\n");
	Log_Debug("\r\n");
	Log_Debug(">When you press Button A, the application will Program the PMOD with the \n");
	Log_Debug("application that has been included with this demonstration software.\n\n");
	Log_Debug(">When you press Button B, the PMOD will be reset and the programmed \n");
	Log_Debug("applicatioin will be started.\n\n");
		
    if (InitPeripheralsAndHandlers() != 0) {
        terminationRequired = true;
    }

    // Use epoll to wait for events and trigger handlers, until an error or SIGTERM happens
    while (!terminationRequired) {
        if (WaitForEventAndCallHandler(epollFd) != 0) {
            terminationRequired = true;
        }
    }

    ClosePeripheralsAndHandlers();
    Log_Debug("Application exiting\n");
    return 0;
}
