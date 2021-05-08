
#include <applibs/gpio.h>
#include <applibs/eventloop.h>
#include <applibs/networking.h>
#include <applibs/log.h>

// Azure IoT SDK
#include <azure_sphere_provisioning.h>
#include <iothub_device_client_ll.h>
#include <iothub_message.h>

#include "hw/joyitcar_appliance.h"

#include "utils.h"
#include "eventloop_timer_utilities.h"

#include "azure_iot_client.h"
#include "i2c_motor_driver.h"

static const char networkInterface[] = "wlan0";

static IOTHUB_DEVICE_CLIENT_LL_HANDLE iothubClientHandle = NULL;

static EventLoopTimer *azureTimer = NULL;

/// <summary>
/// Authentication state of the client with respect to the Azure IoT Hub.
/// </summary>
typedef enum
{
    /// <summary>Client is not authenticated by the Azure IoT Hub.</summary>
    IoTHubClientAuthenticationState_NotAuthenticated = 0,
    /// <summary>Client has initiated authentication to the Azure IoT Hub.</summary>
    IoTHubClientAuthenticationState_AuthenticationInitiated = 1,
    /// <summary>Client is authenticated by the Azure IoT Hub.</summary>
    IoTHubClientAuthenticationState_Authenticated = 2
} IoTHubClientAuthenticationState;

// Authentication state with respect to the IoT Hub.
static IoTHubClientAuthenticationState iotHubClientAuthenticationState = IoTHubClientAuthenticationState_NotAuthenticated;

// Azure IoT poll periods
static const int AzureIoTDefaultPollPeriodSeconds = 10;       // poll azure iot every second
static const int AzureIoTMinReconnectPeriodSeconds = 10;      // back off when reconnecting
static const int AzureIoTMaxReconnectPeriodSeconds = 10 * 60; // back off limit

static int azureIoTPollPeriodSeconds = -1;

static int blueLedFd = -1, redLedFd = -1, greenLedFd = -1;

/// <summary>
///     Converts AZURE_SPHERE_PROV_RETURN_VALUE to a string.
/// </summary>
static const char *GetAzureSphereProvisioningResultString(
    AZURE_SPHERE_PROV_RETURN_VALUE provisioningResult)
{
    switch (provisioningResult.result)
    {
    case AZURE_SPHERE_PROV_RESULT_OK:
        return "AZURE_SPHERE_PROV_RESULT_OK";
    case AZURE_SPHERE_PROV_RESULT_INVALID_PARAM:
        return "AZURE_SPHERE_PROV_RESULT_INVALID_PARAM";
    case AZURE_SPHERE_PROV_RESULT_NETWORK_NOT_READY:
        return "AZURE_SPHERE_PROV_RESULT_NETWORK_NOT_READY";
    case AZURE_SPHERE_PROV_RESULT_DEVICEAUTH_NOT_READY:
        return "AZURE_SPHERE_PROV_RESULT_DEVICEAUTH_NOT_READY";
    case AZURE_SPHERE_PROV_RESULT_PROV_DEVICE_ERROR:
        return "AZURE_SPHERE_PROV_RESULT_PROV_DEVICE_ERROR";
    case AZURE_SPHERE_PROV_RESULT_GENERIC_ERROR:
        return "AZURE_SPHERE_PROV_RESULT_GENERIC_ERROR";
    default:
        return "UNKNOWN_RETURN_VALUE";
    }
}

/// <summary>
///     Converts the Azure IoT Hub connection status reason to a string.
/// </summary>
static const char *GetReasonString(IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason)
{
    static char *reasonString = "unknown reason";
    switch (reason)
    {
    case IOTHUB_CLIENT_CONNECTION_EXPIRED_SAS_TOKEN:
        reasonString = "IOTHUB_CLIENT_CONNECTION_EXPIRED_SAS_TOKEN";
        break;
    case IOTHUB_CLIENT_CONNECTION_DEVICE_DISABLED:
        reasonString = "IOTHUB_CLIENT_CONNECTION_DEVICE_DISABLED";
        break;
    case IOTHUB_CLIENT_CONNECTION_BAD_CREDENTIAL:
        reasonString = "IOTHUB_CLIENT_CONNECTION_BAD_CREDENTIAL";
        break;
    case IOTHUB_CLIENT_CONNECTION_RETRY_EXPIRED:
        reasonString = "IOTHUB_CLIENT_CONNECTION_RETRY_EXPIRED";
        break;
    case IOTHUB_CLIENT_CONNECTION_NO_NETWORK:
        reasonString = "IOTHUB_CLIENT_CONNECTION_NO_NETWORK";
        break;
    case IOTHUB_CLIENT_CONNECTION_COMMUNICATION_ERROR:
        reasonString = "IOTHUB_CLIENT_CONNECTION_COMMUNICATION_ERROR";
        break;
    case IOTHUB_CLIENT_CONNECTION_OK:
        reasonString = "IOTHUB_CLIENT_CONNECTION_OK";
        break;
    case IOTHUB_CLIENT_CONNECTION_NO_PING_RESPONSE:
        reasonString = "IOTHUB_CLIENT_CONNECTION_NO_PING_RESPONSE";
        break;
    }
    return reasonString;
}

/// <summary>
///     Check the network status.
/// </summary>
static bool IsConnectionReadyToSendTelemetry(void)
{
    Networking_InterfaceConnectionStatus status;
    if (Networking_GetInterfaceConnectionStatus(networkInterface, &status) != 0)
    {
        if (errno != EAGAIN)
        {
            Log_Debug("ERROR: Networking_GetInterfaceConnectionStatus: %d (%s)\n", errno, strerror(errno));
            exitCode = IoTDevice_ExitCode_InterfaceConnectionStatus_Failed;
            return false;
        }
        Log_Debug(
            "WARNING: Cannot send Azure IoT Hub telemetry because the networking stack isn't ready "
            "yet.\n");
        return false;
    }

    if ((status & Networking_InterfaceConnectionStatus_ConnectedToInternet) == 0)
    {
        Log_Debug(
            "WARNING: Cannot send Azure IoT Hub telemetry because the device is not connected to "
            "the internet.\n");
        return false;
    }

    return true;
}

/// <summary>
///     Callback invoked when the Device Twin report state request is processed by Azure IoT Hub
///     client.
/// </summary>
static void ReportedStateCallback(int result, void *context)
{
    Log_Debug("INFO: Azure IoT Hub Device Twin reported state callback: status code %d.\n", result);
}

/// <summary>
///     Enqueues a report containing Device Twin reported properties. The report is not sent
///     immediately, but it is sent on the next invocation of IoTHubDeviceClient_LL_DoWork().
/// </summary>
static void TwinReportState(const char *jsonState)
{
    if (iothubClientHandle == NULL)
    {
        Log_Debug("ERROR: Azure IoT Hub client not initialized.\n");
    }
    else
    {
        if (IoTHubDeviceClient_LL_SendReportedState(
                iothubClientHandle, (const unsigned char *)jsonState, strlen(jsonState),
                ReportedStateCallback, NULL) != IOTHUB_CLIENT_OK)
        {
            Log_Debug("ERROR: Azure IoT Hub client error when reporting state '%s'.\n", jsonState);
        }
        else
        {
            Log_Debug("INFO: Azure IoT Hub client accepted request to report state '%s'.\n",
                      jsonState);
        }
    }
}

/// <summary>
///     Callback when the Azure IoT connection state changes.
///     This can indicate that a new connection attempt has succeeded or failed.
///     It can also indicate than an existing connection has expired due to SAS token expiry.
/// </summary>
static void ConnectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result,
                                     IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason,
                                     void *userContextCallback)
{
    Log_Debug("Azure IoT connection status: %s\n", GetReasonString(reason));

    if (result != IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
    {
        iotHubClientAuthenticationState = IoTHubClientAuthenticationState_NotAuthenticated;
        return;
    }

    iotHubClientAuthenticationState = IoTHubClientAuthenticationState_Authenticated;

    // Send static device twin properties when connection is established.
    TwinReportState("{\"manufacturer\":\"Kevin BEAUGRAND\",\"model\":\"Joy-It Car\"}");
}

/// <summary>
///     Callback invoked when the Azure IoT Hub send event request is processed.
/// </summary>
static void SendEventCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *context)
{
    Log_Debug("INFO: Azure IoT Hub send telemetry event callback: status code %d.\n", result);

    // GPIO_SetValue(blueLedFd, GPIO_Value_High);
    // GPIO_SetValue(redLedFd, GPIO_Value_High);

    // GPIO_SetValue(greenLedFd, GPIO_Value_Low);

    // struct timespec wait = {.tv_nsec = 50 * 1000 * 1000};

    // nanosleep(&wait, &wait);

    // GPIO_SetValue(greenLedFd, GPIO_Value_High);
}

/// <summary>
///     Sends telemetry to Azure IoT Hub
/// </summary>
void SendTelemetry(const char *jsonMessage)
{
    if (iotHubClientAuthenticationState != IoTHubClientAuthenticationState_Authenticated)
    {
        // AzureIoT client is not authenticated. Log a warning and return.
        Log_Debug("WARNING: Azure IoT Hub is not authenticated. Not sending telemetry.\n");
        return;
    }

    Log_Debug("Sending Azure IoT Hub telemetry: %s.\n", jsonMessage);

    // Check whether the device is connected to the internet.
    if (IsConnectionReadyToSendTelemetry() == false)
    {
        return;
    }

    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromString(jsonMessage);

    if (messageHandle == 0)
    {
        Log_Debug("ERROR: unable to create a new IoTHubMessage.\n");
        return;
    }

    if (IoTHubDeviceClient_LL_SendEventAsync(iothubClientHandle, messageHandle, SendEventCallback,
                                             /*&callback_param*/ NULL) != IOTHUB_CLIENT_OK)
    {
        Log_Debug("ERROR: failure requesting IoTHubClient to send telemetry event.\n");
    }
    else
    {
        Log_Debug("INFO: IoTHubClient accepted the telemetry event for delivery.\n");
    }

    IoTHubMessage_Destroy(messageHandle);
}

/// <summary>
///     Callback invoked when a Direct Method is received from Azure IoT Hub.
/// </summary>
static int DeviceMethodCallback(const char *methodName, const unsigned char *payload,
                                size_t payloadSize, unsigned char **response, size_t *responseSize,
                                void *userContextCallback)
{
    // All methods are ignored
    int result = 0;
    char *responseString = "{\"result\":\"OK\"}";

    Log_Debug("Received Device Method callback: Method name %s.\n", methodName);

    // if 'response' is non-NULL, the Azure IoT library frees it after use, so copy it to heap
    *responseSize = strlen(responseString);
    *response = malloc(*responseSize);
    memcpy(*response, responseString, *responseSize);

    if (strcmp(methodName, "GoForward") == 0)
    {
        JoyitCar_GoForward();
    }
    else if (strcmp(methodName, "GoBackward") == 0)
    {
        JoyitCar_GoBackward();
    }
    else if (strcmp(methodName, "Break") == 0)
    {
        JoyitCar_Break();
    }
    else if (strcmp(methodName, "TurnRight") == 0)
    {
        JoyitCar_TurnRight();
    }
    else if (strcmp(methodName, "TurnLeft") == 0)
    {
        JoyitCar_TurnLeft();
    }
    else if (strcmp(methodName, "StartDemo") == 0)
    {
        JoyitCar_StartDemo();
    }
    else
    {
        responseString = "{\"result\":\"NotFound\"}";
        result = -1;
    }

    return result;
}

/// <summary>
///     Sets up the Azure IoT Hub connection (creates the iothubClientHandle)
///     When the SAS Token for a device expires the connection needs to be recreated
///     which is why this is not simply a one time call.
/// </summary>
static void SetUpAzureIoTHubClient(void)
{
    bool isClientSetupSuccessful = false;

    if (iothubClientHandle != NULL)
    {
        IoTHubDeviceClient_LL_Destroy(iothubClientHandle);
    }

    AZURE_SPHERE_PROV_RETURN_VALUE provResult =
        IoTHubDeviceClient_LL_CreateWithAzureSphereDeviceAuthProvisioning(
            "0ne000E0092", 10000, &iothubClientHandle);
    Log_Debug("IoTHubDeviceClient_LL_CreateWithAzureSphereDeviceAuthProvisioning returned '%s'.\n",
              GetAzureSphereProvisioningResultString(provResult));

    if (provResult.result != AZURE_SPHERE_PROV_RESULT_OK)
    {
        isClientSetupSuccessful = false;
    }
    else
    {
        isClientSetupSuccessful = true;
    }

    if (!isClientSetupSuccessful)
    {
        // If we fail to connect, reduce the polling frequency, starting at
        // AzureIoTMinReconnectPeriodSeconds and with a backoff up to
        // AzureIoTMaxReconnectPeriodSeconds
        if (azureIoTPollPeriodSeconds == AzureIoTDefaultPollPeriodSeconds)
        {
            azureIoTPollPeriodSeconds = AzureIoTMinReconnectPeriodSeconds;
        }
        else
        {
            azureIoTPollPeriodSeconds *= 2;
            if (azureIoTPollPeriodSeconds > AzureIoTMaxReconnectPeriodSeconds)
            {
                azureIoTPollPeriodSeconds = AzureIoTMaxReconnectPeriodSeconds;
            }
        }

        struct timespec azureTelemetryPeriod = {azureIoTPollPeriodSeconds, 0};
        SetEventLoopTimerPeriod(azureTimer, &azureTelemetryPeriod);

        Log_Debug("ERROR: Failed to create IoTHub Handle - will retry in %i seconds.\n",
                  azureIoTPollPeriodSeconds);
        return;
    }

    // Successfully connected, so make sure the polling frequency is back to the default
    azureIoTPollPeriodSeconds = AzureIoTDefaultPollPeriodSeconds;
    struct timespec azureTelemetryPeriod = {.tv_sec = azureIoTPollPeriodSeconds, .tv_nsec = 0};
    SetEventLoopTimerPeriod(azureTimer, &azureTelemetryPeriod);

    // Set client authentication state to initiated. This is done to indicate that
    // SetUpAzureIoTHubClient() has been called (and so should not be called again) while the
    // client is waiting for a response via the ConnectionStatusCallback().
    iotHubClientAuthenticationState = IoTHubClientAuthenticationState_AuthenticationInitiated;

    // IoTHubDeviceClient_LL_SetDeviceTwinCallback(iothubClientHandle, DeviceTwinCallback, NULL);
    IoTHubDeviceClient_LL_SetDeviceMethodCallback(iothubClientHandle, DeviceMethodCallback, NULL);
    IoTHubDeviceClient_LL_SetConnectionStatusCallback(iothubClientHandle, ConnectionStatusCallback,
                                                      NULL);
}

/// <summary>
///     Azure timer event:  Check connection status and send telemetry
/// </summary>
static void AzureTimerEventHandler(EventLoopTimer *timer)
{
    if (ConsumeEventLoopTimerEvent(timer) != 0)
    {
        exitCode = IoTDevice_ExitCode_AzureTimer_Consume;
        return;
    }

    // Check whether the device is connected to the internet.
    Networking_InterfaceConnectionStatus status;
    if (Networking_GetInterfaceConnectionStatus(networkInterface, &status) == 0)
    {
        if ((status & Networking_InterfaceConnectionStatus_ConnectedToInternet) &&
            (iotHubClientAuthenticationState == IoTHubClientAuthenticationState_NotAuthenticated))
        {
            SetUpAzureIoTHubClient();
        }
    }
    else
    {
        if (errno != EAGAIN)
        {
            Log_Debug("ERROR: Networking_GetInterfaceConnectionStatus: %d (%s)\n", errno,
                      strerror(errno));
            exitCode = IoTDevice_ExitCode_InterfaceConnectionStatus_Failed;
            return;
        }
    }

    if (iotHubClientAuthenticationState == IoTHubClientAuthenticationState_Authenticated)
    {
        GPIO_SetValue(blueLedFd, GPIO_Value_High);
        GPIO_SetValue(greenLedFd, GPIO_Value_Low);
        GPIO_SetValue(redLedFd, GPIO_Value_High);
        
        SendTelemetry("{\"txt\":\"Hello World\"}");
    }

    if (iothubClientHandle != NULL)
    {
        IoTHubDeviceClient_LL_DoWork(iothubClientHandle);
    }
}

IoTDevice_ExitCode JoyitCar_InitAzureIoT(EventLoop *eventLoop)
{
    // Open RGBLED_BLUE GPIO and set as output with value GPIO_Value_High (off).
    Log_Debug("Opening RGBLED_BLUE\n");
    blueLedFd =
        GPIO_OpenAsOutput(RGBLED_BLUE, GPIO_OutputMode_PushPull, GPIO_Value_High);
    if (blueLedFd == -1)
    {
        Log_Debug("ERROR: Could not open RGBLED_BLUE GPIO: %s (%d).\n", strerror(errno),
                  errno);
        return IoTDevice_ExitCode_Init_BlueUserLed;
    }

    // Open RGBLED_RED GPIO and set as output with value GPIO_Value_High (off).
    Log_Debug("Opening RGBLED_RED\n");
    redLedFd =
        GPIO_OpenAsOutput(RGBLED_RED, GPIO_OutputMode_PushPull, GPIO_Value_High);
    if (redLedFd == -1)
    {
        Log_Debug("ERROR: Could not open RGBLED_RED GPIO: %s (%d).\n", strerror(errno),
                  errno);
        return IoTDevice_ExitCode_Init_RedUserLed;
    }

    // Open RGBLED_GREEN GPIO and set as output with value GPIO_Value_High (off).
    Log_Debug("Opening RGBLED_GREEN\n");
    greenLedFd =
        GPIO_OpenAsOutput(RGBLED_GREEN, GPIO_OutputMode_PushPull, GPIO_Value_High);
    if (greenLedFd == -1)
    {
        Log_Debug("ERROR: Could not open RGBLED_GREEN GPIO: %s (%d).\n", strerror(errno),
                  errno);
        return IoTDevice_ExitCode_Init_GreenUserLed;
    }

    azureIoTPollPeriodSeconds = AzureIoTDefaultPollPeriodSeconds;
    struct timespec azureTelemetryPeriod = {.tv_sec = azureIoTPollPeriodSeconds, .tv_nsec = 0};
    azureTimer = CreateEventLoopPeriodicTimer(eventLoop, &AzureTimerEventHandler, &azureTelemetryPeriod);

    if (azureTimer == NULL)
    {
        return IoTDevice_ExitCode_Init_AzureTimer;
    }

    return IoTDevice_ExitCode_Success;
}