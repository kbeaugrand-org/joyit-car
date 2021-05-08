
#include <time.h>
#include <errno.h>
#include <string.h>

#include <applibs/log.h>
#include <applibs/gpio.h>

#include "hw/joyitcar_appliance.h"
#include "eventloop_timer_utilities.h"
#include "utils.h"

#include "button_behavior.h"
#include "i2c_motor_driver.h"

static int userButtonAFd = -1, userButtonBFd = -1;
static GPIO_Value_Type userButtonAState = GPIO_Value_High, userButtonBState = GPIO_Value_High;
static EventLoopTimer *userButtonAPollTimer = NULL, *userButtonBPollTimer = NULL;

/// <summary>
///     Check whether a given button has just been pressed.
/// </summary>
/// <param name="fd">The button file descriptor</param>
/// <param name="oldState">Old state of the button (pressed or released)</param>
/// <returns>true if pressed, false otherwise</returns>
static bool IsButtonPressed(int fd, GPIO_Value_Type *oldState)
{
    GPIO_Value_Type newState;
    int result = GPIO_GetValue(fd, &newState);
    if (result != 0)
    {
        Log_Debug("ERROR: Could not read button GPIO: %s (%d).\n", strerror(errno), errno);
        exitCode = -1;
        return false;
    }

    // Button is pressed if it is low and different than last known state.
    if ((newState == *oldState) || (newState != GPIO_Value_Low))
    {
        return false;
    }

    *oldState = newState;

    return true;
}

/// <summary>
///     Check whether a given button has just been leased.
/// </summary>
/// <param name="fd">The button file descriptor</param>
/// <param name="oldState">Old state of the button (pressed or released)</param>
/// <returns>true if leased, false otherwise</returns>
static bool IsButtonLeased(int fd, GPIO_Value_Type *oldState)
{
    GPIO_Value_Type newState;
    int result = GPIO_GetValue(fd, &newState);
    if (result != 0)
    {
        Log_Debug("ERROR: Could not read button GPIO: %s (%d).\n", strerror(errno), errno);
        exitCode = -1;
        return false;
    }

    // Button is released if it is high and different than last known state.
    if ((newState == *oldState) || (newState != GPIO_Value_High))
    {
        return false;
    }

    *oldState = newState;

    return true;
}

/// <summary>
///     Handle button timer event: if the button is pressed, change the LED blink rate.
/// </summary>
static void ButtonATimerEventHandler(EventLoopTimer *timer)
{
    if (ConsumeEventLoopTimerEvent(timer) != 0)
    {
        exitCode = -1;
        return;
    }

    if (IsButtonPressed(userButtonAFd, &userButtonAState))
    {
        JoyitCar_GoForward();
    }

    if (IsButtonLeased(userButtonAFd, &userButtonAState))
    {
        JoyitCar_Break();
    }
}

/// <summary>
///     Handle button timer event: if the button is pressed, change the LED blink rate.
/// </summary>
static void ButtonBTimerEventHandler(EventLoopTimer *timer)
{
    if (ConsumeEventLoopTimerEvent(timer) != 0)
    {
        exitCode = -1;
        return;
    }

    if (IsButtonPressed(userButtonBFd, &userButtonBState))
    {
        JoyitCar_GoBackward();
    }

    if (IsButtonLeased(userButtonBFd, &userButtonBState))
    {
        JoyitCar_Break();
    }
}

ButtonBehaviors_ExitCode JoyitCar_InitButtonsAndHandlers(EventLoop *eventLoop)
{
    struct timespec buttonPressCheckPeriod = {.tv_sec = 0, .tv_nsec = 200000};

    // Open USER_BUTTON_A GPIO as input, and set up a timer to poll it
    Log_Debug("Opening USER_BUTTON_A as input.\n");
    userButtonAFd = GPIO_OpenAsInput(USER_BUTTON_A);
    if (userButtonAFd == -1)
    {
        Log_Debug("ERROR: Could not open USER_BUTTON_A: %s (%d).\n", strerror(errno), errno);

        return ButtonBehaviors_ExitCode_OpenButtonAError;
    }
    userButtonAPollTimer =
        CreateEventLoopPeriodicTimer(eventLoop, &ButtonATimerEventHandler, &buttonPressCheckPeriod);
    if (userButtonAPollTimer == NULL)
    {
        return ButtonBehaviors_ExitCode_InitButtonATimer;
    }

    // Open USER_BUTTON_B GPIO as input, and set up a timer to poll it
    Log_Debug("Opening USER_BUTTON_B as input.\n");
    userButtonBFd = GPIO_OpenAsInput(USER_BUTTON_B);
    if (userButtonBFd == -1)
    {
        Log_Debug("ERROR: Could not open USER_BUTTON_B: %s (%d).\n", strerror(errno), errno);
        return ButtonBehaviors_ExitCode_OpenButtonBError;
    }
    userButtonBPollTimer =
        CreateEventLoopPeriodicTimer(eventLoop, &ButtonBTimerEventHandler, &buttonPressCheckPeriod);
    if (userButtonAPollTimer == NULL)
    {
        return ButtonBehaviors_ExitCode_InitButtonBTimer;
    }

    return ButtonBehaviors_ExitCode_Success;
}
