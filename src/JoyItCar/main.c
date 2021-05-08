// This minimal Azure Sphere app prints "High Level Application" to the debug
// console and exits with status 0.

#include <stdbool.h>
#include <errno.h>
#include <sys/socket.h>
#include <string.h>

#include <applibs/log.h>
#include <applibs/gpio.h>
#include <applibs/application.h>
#include <applibs/eventloop.h>
#include <applibs/networking.h>

#include "hw/joyitcar_appliance.h"

#include "utils.h"
#include "eventloop_timer_utilities.h"

#include "button_behavior.h"
#include "i2c_motor_driver.h"
#include "azure_iot_client.h"
#include "ble_commands.h"

/// <summary>
/// Exit codes for this application. These are used for the
/// application exit code. They must all be between zero and 255,
/// where zero is reserved for successful termination.
/// </summary>
typedef enum
{
    ExitCode_Success = 0,
    ExitCode_TermHandler_SigTerm = 1,
    ExitCode_Init_EventLoop = 2,
    ExitCode_Main_EventLoopFail = 3,

} ExitCode;

static EventLoop *eventLoop = NULL;

/// <summary>
///     Signal handler for termination requests. This handler must be async-signal-safe.
/// </summary>
static void TerminationHandler(int signalNumber)
{
    // Don't use Log_Debug here, as it is not guaranteed to be async-signal-safe.
    exitCode = ExitCode_TermHandler_SigTerm;
}

/// <summary>
///     Set up SIGTERM termination handler, initialize peripherals, and set up event handlers.
/// </summary>
/// <returns>
///     ExitCode_Success if all resources were allocated successfully; otherwise another
///     ExitCode value which indicates the specific failure.
/// </returns>
static ExitCode InitPeripheralsAndHandlers(void)
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

    eventLoop = EventLoop_Create();
    if (eventLoop == NULL)
    {
        Log_Debug("Could not create event loop.\n");
        return ExitCode_Init_EventLoop;
    }

    I2CMotorDriverExitCode motorsInitResult = JoyitCar_InitMotors();

    if (motorsInitResult != I2CMotorDriver_ExitCode_Success)
    {
        return motorsInitResult;
    }

    ButtonBehaviors_ExitCode buttonInitResult = JoyitCar_InitButtonsAndHandlers(eventLoop);

    if (buttonInitResult != ButtonBehaviors_ExitCode_Success)
    {
        return buttonInitResult;
    }

    BLECommands_ExitCode bleCommandInitResult = JoyitCar_InitBLECommandHandlers(eventLoop);

    if (bleCommandInitResult != BLECommands_ExitCode_Success)
    {
        return bleCommandInitResult;
    }

    JoyitCar_InitAzureIoT(eventLoop);

    return ExitCode_Success;
}


/// <summary>
///     Close peripherals and handlers.
/// </summary>
static void ClosePeripheralsAndHandlers(void)
{
    /// Dispose event loops
    EventLoop_Close(eventLoop);

    JoyitCar_CloseMotors();
}

int main(int argc, char *argv[])
{
    Log_Debug("Application starting.\n");

    bool isNetworkingReady = false;
    if ((Networking_IsNetworkingReady(&isNetworkingReady) == -1) || !isNetworkingReady)
    {
        Log_Debug("WARNING: Network is not ready. Device cannot connect until network is ready.\n");
    }

    exitCode = InitPeripheralsAndHandlers();

    // Main loop
    while (exitCode == ExitCode_Success)
    {
        EventLoop_Run_Result result = EventLoop_Run(eventLoop, -1, true);
        // Continue if interrupted by signal, e.g. due to breakpoint being set.
        if (result == EventLoop_Run_Failed && errno != EINTR)
        {
            exitCode = ExitCode_Main_EventLoopFail;
        }
    }

    ClosePeripheralsAndHandlers();

    return exitCode;
}