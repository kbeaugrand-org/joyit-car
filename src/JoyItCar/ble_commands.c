
#include <string.h>
#include <unistd.h>

#include <applibs/uart.h>
#include <applibs/log.h>

#include "libs/ble4_click/ble4.h"
#include "eventloop_timer_utilities.h"

#include "ble_commands.h"
#include "i2c_motor_driver.h"

#define PROCESS_COUNTER 5
#define PROCESS_RX_BUFFER_SIZE 100
#define PROCESS_PARSER_BUFFER_SIZE 100

static ble4_t ble4;

static char current_parser_buf[PROCESS_PARSER_BUFFER_SIZE];

static EventLoopTimer *bleCommandPollTimer = NULL;
static struct timespec runDuration = {.tv_sec = 0, .tv_nsec = 1000 * 1000 * 100};

static void Delay_ms(int ms)
{
    static struct timespec delay = {.tv_sec = 0, .tv_nsec = 0};
    delay.tv_nsec = 1000 * 1000 * ms;

    nanosleep(&delay, NULL);
}

static void Delay_100ms(void)
{
    Delay_ms(100);
}

static void Delay_1sec(void)
{
    Delay_ms(1000);
}

static int8_t ble4_process(void)
{
    size_t rsp_size;
    size_t rsp_cnt = 0;

    char uart_rx_buffer[PROCESS_RX_BUFFER_SIZE] = {0};
    uint8_t check_buf_cnt;
    uint8_t process_cnt = PROCESS_COUNTER;

    // Clear current buffer
    memset(current_parser_buf, 0, PROCESS_PARSER_BUFFER_SIZE);

    while (process_cnt != 0)
    {
        rsp_size = ble4_generic_read(&ble4, uart_rx_buffer, PROCESS_RX_BUFFER_SIZE);

        if (rsp_size > 0)
        {
            // Validation of the received data
            for (check_buf_cnt = 0; check_buf_cnt < rsp_size; check_buf_cnt++)
            {
                if (uart_rx_buffer[check_buf_cnt] == 0)
                {
                    uart_rx_buffer[check_buf_cnt] = 13;
                }
            }
            // Storages data in current buffer
            rsp_cnt += rsp_size;
            if (rsp_cnt < PROCESS_PARSER_BUFFER_SIZE)
            {
                strncat(current_parser_buf, uart_rx_buffer, rsp_size);
            }

            // Clear RX buffer
            memset(uart_rx_buffer, 0, PROCESS_RX_BUFFER_SIZE);

            if (strstr(current_parser_buf, "ERROR"))
            {
                return -1;
            }

            return 1;
        }
        else
        {
            process_cnt--;

            // Process delay
            Delay_ms(100);
        }
    }

    return 0;
}

static bool has_data(void)
{
    return ble4_process() == 1;
}

static void BLECommandTimerEventHandler(EventLoopTimer *timer)
{
    if (ConsumeEventLoopTimerEvent(timer) != 0)
    {
        return;
    }

    if (!has_data())
    {
        return;
    }

    Log_Debug("%s\n", current_parser_buf);

    if (strcmp(current_parser_buf, "Forward") == 0)
    {
        JoyitCar_GoForward();
    }
    else if (strcmp(current_parser_buf, "Backward") == 0)
    {
        JoyitCar_GoBackward();
    }
    else if (strcmp(current_parser_buf, "Break") == 0)
    {
        JoyitCar_Break();
    }
    else if (strcmp(current_parser_buf, "Right") == 0)
    {
        JoyitCar_TurnRight();
    }
    else if (strcmp(current_parser_buf, "Left") == 0)
    {
        JoyitCar_TurnLeft();
    }

    nanosleep(&runDuration, &runDuration);

    JoyitCar_Break();
}

BLECommands_ExitCode JoyitCar_InitBLECommandHandlers(EventLoop *eventLoop)
{
    struct timespec bleCheckPeriod = {.tv_sec = 0, .tv_nsec = 1000 * 1000 * 50};

    if (ble4_init(&ble4) != BLE4_OK)
    {
        return BLECommands_ExitCode_Initevice;
    }

    ble4_reset(&ble4);
    Delay_1sec();

    Log_Debug("Configuring the BLE module...\n");
    Delay_1sec();

    ble4_set_dsr_pin(&ble4, 1);
    Delay_ms(20);

    do
    {
        ble4_set_echo_cmd(&ble4, 1);
        Delay_100ms();
    } while (ble4_process() != 1);

    do
    {
        ble4_set_local_name_cmd(&ble4, "JoyItCar");
        Delay_100ms();
    } while (ble4_process() != 1);

    do
    {
        ble4_connectability_en_cmd(&ble4, BLE4_GAP_CONNECTABLE_MODE);
        Delay_100ms();
    } while (ble4_process() != 1);

    do
    {
        ble4_discoverability_en_cmd(&ble4, BLE4_GAP_GENERAL_DISCOVERABLE_MODE);
        Delay_100ms();
    } while (ble4_process() != 1);

    do
    {
        ble4_enter_mode_cmd(&ble4, BLE4_DATA_MODE);
        Delay_100ms();
    } while (ble4_process() != 1);

    ble4_set_dsr_pin(&ble4, 0);
    Delay_ms(20);
    Log_Debug("The BLE module has been configured.\n");

    bleCommandPollTimer =
        CreateEventLoopPeriodicTimer(eventLoop, &BLECommandTimerEventHandler, &bleCheckPeriod);
    if (bleCommandPollTimer == NULL)
    {
        return BLECommands_ExitCode_InitTimer;
    }

    return BLECommands_ExitCode_Success;
}