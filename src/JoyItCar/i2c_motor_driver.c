#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <applibs/i2c.h>
#include <applibs/log.h>

#include "i2c_motor_driver.h"
#include "hw/joyitcar_appliance.h"

static uint8_t _buffer[3];
static int i2cFd = -1;

typedef enum MotorChannel {
    MOTOR_CHA = 0,
    MOTOR_CHB = 1,
} MotorChannel;

I2CMotorDriverExitCode JoyitCar_InitMotors(void)
{
    i2cFd = I2CMaster_Open(I2C_MOTOR_DRIVER);
    if (i2cFd == -1) {
        Log_Debug("ERROR: I2CMaster_Open: errno=%d (%s)\n", errno, strerror(errno));
        return I2CMotorDriver_ExitCode_Init_OpenMaster;
    }


    int result = I2CMaster_SetBusSpeed(i2cFd, I2C_BUS_SPEED_STANDARD);
    if (result != 0) {
        Log_Debug("ERROR: I2CMaster_SetBusSpeed: errno=%d (%s)\n", errno, strerror(errno));
        return I2CMotorDriver_ExitCode_Init_SetBusSpeed;
    }

    result = I2CMaster_SetTimeout(i2cFd, 100);
    if (result != 0) {
        Log_Debug("ERROR: I2CMaster_SetTimeout: errno=%d (%s)\n", errno, strerror(errno));
        return I2CMotorDriver_ExitCode_Init_SetTimeout;
    }

    return I2CMotorDriver_ExitCode_Success;
}

void JoyitCar_CloseMotors(void)
{    
    if (i2cFd == -1)
    {
        return;
    }

    int result = close(i2cFd);

    if (result != 0) {
        Log_Debug("ERROR: Could not close fd I2CMaster: %s (%d).\n", strerror(errno), errno);
    }
}

static void JoyitCar_StartMotor(MotorChannel channel, int speed)
{
    if(speed > 0)
    {
        _buffer[0] = GROVE_MOTOR_DRIVER_I2C_CMD_CW;
        _buffer[2] = (uint8_t )speed;
    }
    else
    {
        _buffer[0] = GROVE_MOTOR_DRIVER_I2C_CMD_CCW;
        _buffer[2] = (uint8_t) -speed;
    }

    _buffer[1] = channel;

    Log_Debug("INFO: Starting (%d) motor %d with speed %d\n", _buffer[0], channel, _buffer[2]);

    ssize_t written = I2CMaster_Write(i2cFd, GROVE_MOTOR_DRIVER_DEFAULT_I2C_ADDR, _buffer, sizeof(_buffer));

    if (written < 0)
    {
        Log_Debug("ERROR: Failed to write to I2C (0x%d). Writing %d bytes ...\n", GROVE_MOTOR_DRIVER_DEFAULT_I2C_ADDR, sizeof(_buffer));
    }
}

static void JoyitCar_StopMotor(MotorChannel channel)
{
    _buffer[0] = GROVE_MOTOR_DRIVER_I2C_CMD_STOP;
    _buffer[1] = channel;

    Log_Debug("INFO: Stoping motor %d\n", channel);

    ssize_t written = I2CMaster_Write(i2cFd, GROVE_MOTOR_DRIVER_DEFAULT_I2C_ADDR, _buffer, sizeof(_buffer) - 1);

    if (written < 0)
    {
        Log_Debug("ERROR: Failed to write to I2C (0x%d). Writing %d bytes ...\n", GROVE_MOTOR_DRIVER_DEFAULT_I2C_ADDR, sizeof(_buffer) - 1);
    }
}

void JoyitCar_GoForward(void)
{
    JoyitCar_StartMotor(MOTOR_CHA, DEFAULT_MOTOR_SPEED);
    JoyitCar_StartMotor(MOTOR_CHB, DEFAULT_MOTOR_SPEED);
}

void JoyitCar_GoBackward(void)
{
    JoyitCar_StartMotor(MOTOR_CHA, -DEFAULT_MOTOR_SPEED);
    JoyitCar_StartMotor(MOTOR_CHB, -DEFAULT_MOTOR_SPEED);
}

void JoyitCar_TurnLeft(void)
{
    JoyitCar_StartMotor(MOTOR_CHA, -DEFAULT_MOTOR_SPEED);
    JoyitCar_StartMotor(MOTOR_CHB, DEFAULT_MOTOR_SPEED);
}

void JoyitCar_TurnRight(void)
{
    JoyitCar_StartMotor(MOTOR_CHA, DEFAULT_MOTOR_SPEED);
    JoyitCar_StartMotor(MOTOR_CHB, -DEFAULT_MOTOR_SPEED);
}

void JoyitCar_Break(void)
{
    JoyitCar_StopMotor(MOTOR_CHA);
    JoyitCar_StopMotor(MOTOR_CHB);
}

void JoyitCar_StartDemo(void)
{
    struct timespec wait = {.tv_nsec = 250000000};

    JoyitCar_GoForward();
    sleep(1);
    JoyitCar_Break();
    nanosleep(&wait, &wait);

    JoyitCar_GoBackward();
    sleep(1);
    JoyitCar_Break();
    nanosleep(&wait, &wait);

    JoyitCar_TurnRight();
    nanosleep(&wait, &wait);
    JoyitCar_Break();
    nanosleep(&wait, &wait);

    JoyitCar_TurnLeft();
    nanosleep(&wait, &wait);
    nanosleep(&wait, &wait);
    JoyitCar_Break();
    nanosleep(&wait, &wait);

    JoyitCar_TurnRight();
    nanosleep(&wait, &wait);
    JoyitCar_Break();
}