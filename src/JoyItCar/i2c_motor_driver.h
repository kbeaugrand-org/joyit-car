#pragma once

#define GROVE_MOTOR_DRIVER_DEFAULT_I2C_ADDR         0x14

#define GROVE_MOTOR_DRIVER_I2C_CMD_BRAKE            0x00
#define GROVE_MOTOR_DRIVER_I2C_CMD_STOP             0x01
#define GROVE_MOTOR_DRIVER_I2C_CMD_CW               0x02
#define GROVE_MOTOR_DRIVER_I2C_CMD_CCW              0x03
#define GROVE_MOTOR_DRIVER_I2C_CMD_STANDBY          0x04
#define GROVE_MOTOR_DRIVER_I2C_CMD_NOT_STANDBY      0x05
#define GROVE_MOTOR_DRIVER_I2C_CMD_STEPPER_RUN      0x06
#define GROVE_MOTOR_DRIVER_I2C_CMD_STEPPER_STOP     0x07
#define GROVE_MOTOR_DRIVER_I2C_CMD_STEPPER_KEEP_RUN 0x08
#define GROVE_MOTOR_DRIVER_I2C_CMD_SET_ADDR         0x11

#define DEFAULT_MOTOR_SPEED 100

typedef enum
{
    I2CMotorDriver_ExitCode_Success = 200,
    I2CMotorDriver_ExitCode_Init_OpenMaster = 201,
    I2CMotorDriver_ExitCode_Init_SetBusSpeed = 202,
    I2CMotorDriver_ExitCode_Init_SetTimeout = 203,
} I2CMotorDriverExitCode;

I2CMotorDriverExitCode JoyitCar_InitMotors(void);

void JoyitCar_CloseMotors(void);

void JoyitCar_GoForward(void);

void JoyitCar_GoBackward(void);

void JoyitCar_TurnLeft(void);

void JoyitCar_TurnRight(void);

void JoyitCar_Break(void);

void JoyitCar_StartDemo(void);