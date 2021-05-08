#pragma once

#include <applibs/eventloop.h>

#include "eventloop_timer_utilities.h"

typedef enum
{
    IoTDevice_ExitCode_Success = 300,
    IoTDevice_ExitCode_InterfaceConnectionStatus_Failed = 301,
    IoTDevice_ExitCode_AzureTimer_Consume = 302,    
    IoTDevice_ExitCode_Init_BlueUserLed = 303,
    IoTDevice_ExitCode_Init_RedUserLed = 304,
    IoTDevice_ExitCode_Init_GreenUserLed = 305,
    IoTDevice_ExitCode_Init_AzureTimer = 306
} IoTDevice_ExitCode;

IoTDevice_ExitCode JoyitCar_InitAzureIoT(EventLoop *eventLoop);

void SendTelemetry(const char *jsonMessage);