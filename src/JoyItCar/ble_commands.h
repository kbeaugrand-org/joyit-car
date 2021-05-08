#pragma once

#include "eventloop_timer_utilities.h"

typedef enum
{
    BLECommands_ExitCode_Success = 500,
    BLECommands_ExitCode_Initevice = 501,
    BLECommands_ExitCode_InitTimer = 502,
    BLECommands_ExitCode_TimerConsume = 503,    
} BLECommands_ExitCode;

BLECommands_ExitCode JoyitCar_InitBLECommandHandlers(EventLoop *eventLoop);