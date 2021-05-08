#pragma once

#include "eventloop_timer_utilities.h"

typedef enum
{
    ButtonBehaviors_ExitCode_Success = 400,
    ButtonBehaviors_ExitCode_OpenButtonAError = 401,
    ButtonBehaviors_ExitCode_OpenButtonBError = 402,
    ButtonBehaviors_ExitCode_InitButtonATimer = 403,
    ButtonBehaviors_ExitCode_InitButtonBTimer = 404,
    ButtonBehaviors_ExitCode_TimerButtonAConsume = 405,    

} ButtonBehaviors_ExitCode;

ButtonBehaviors_ExitCode JoyitCar_InitButtonsAndHandlers(EventLoop *eventLoop);