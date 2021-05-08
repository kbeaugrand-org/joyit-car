#pragma once

/*
 * MikroSDK - MikroE Software Development Kit
 * Copyright© 2020 MikroElektronika d.o.o.
 * 
 * Permission is hereby granted, free of charge, to any person 
 * obtaining a copy of this software and associated documentation 
 * files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, 
 * publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE 
 * OR OTHER DEALINGS IN THE SOFTWARE. 
 */

/*!
 * \file
 *
 * \brief This file contains API for BLE 4  Click driver.
 *
 * \addtogroup ble4 BLE 4  Click Driver
 * @{
 */
// ----------------------------------------------------------------------------

#ifndef BLE4_H
#define BLE4_H

#include <stdbool.h>
#include <applibs/uart.h>

// -------------------------------------------------------------- PUBLIC MACROS
/**
 * \defgroup macros Macros
 * \{
 */

/**
 * \defgroup map_mikrobus MikroBUS
 * \{
 */
#define BLE4_MAP_MIKROBUS(cfg, mikrobus)        \
  cfg.tx_pin = MIKROBUS(mikrobus, MIKROBUS_TX); \
  cfg.rx_pin = MIKROBUS(mikrobus, MIKROBUS_RX); \
  cfg.dtr = MIKROBUS(mikrobus, MIKROBUS_AN);    \
  cfg.rst = MIKROBUS(mikrobus, MIKROBUS_RST);   \
  cfg.cts = MIKROBUS(mikrobus, MIKROBUS_CS);    \
  cfg.dsr = MIKROBUS(mikrobus, MIKROBUS_PWM);   \
  cfg.rts = MIKROBUS(mikrobus, MIKROBUS_INT)
/** \} */

/**
 * \defgroup error_code Error Code
 * \{
 */
#define BLE4_RETVAL uint8_t

#define BLE4_OK 0x00
#define BLE4_INIT_ERROR 0xFF
/** \} */

/**
 * \defgroup term_char  String (data) termination character
 * \{
 */
#define BLE4_END_BUFF 0
/** \} */

/**
 * \defgroup response_states  Response states
 * \{
 */
#define BLE4_RSP_READY 1
#define BLE4_RSP_NOT_READY 0
/** \} */

/**
 * \defgroup settings_for_echo  Settings to turn echo on/off 
 * \{
 */
#define BLE4_ECHO_ON 1
#define BLE4_ECHO_OFF 0
/** \} */

/**
 * \defgroup settings_for_device_mode  Settings to select device mode
 * \{
 */
#define BLE4_COMMAND_MODE 0
#define BLE4_DATA_MODE 1
#define BLE4_EXT_DATA_MODE 2
#define BLE4_PPP_MODE 3
/** \} */

/**
 * \defgroup  device_low_energy_role  Settings to select device low energy role
 * \{
 */
#define BLE4_DISABLED_ROLE 0
#define BLE4_CENTRAL_ROLE 1
#define BLE4_PERIPHERAL_ROLE 2
#define BLE4_SIMULTANEOUS_ROLE 3
/** \} */

/**
 * \defgroup settings_for_security_mode  Settings to select security mode
 * \{
 */
#define BLE4_SEC_DISABLED 1
#define BLE4_SEC_JUST_WORKS 2
#define BLE4_SEC_DISPLAY_ONLY 3
#define BLE4_SEC_DISPLAY_YES_NO 4
#define BLE4_SEC_KEYBOARD_ONLY 5
#define BLE4_SEC_OUT_OF_BAND 6
/** \} */

/**
 * \defgroup settings_for_paring_mode  Settings to enable/disable pairing mode
 * \{
 */
#define BLE4_GAP_NON_PAIRING_MODE 1
#define BLE4_GAP_PAIRING_MODE 2
/** \} */

/**
 * \defgroup settings_for_connectable_mode  Settings to enable/disable connectable mode
 * \{
 */
#define BLE4_GAP_NON_CONNECTABLE_MODE 1
#define BLE4_GAP_CONNECTABLE_MODE 2
/** \} */

/**
 * \defgroup settings_for_discoverable_mode  Settings to enable/disable discoverable mode
 * \{
 */
#define BLE4_GAP_NON_DISCOVERABLE_MODE 1
#define BLE4_GAP_LIMITED_DISCOVERABLE_MODE 2
#define BLE4_GAP_GENERAL_DISCOVERABLE_MODE 3
/** \} */

/**
 * \defgroup driver Driver define
 * \{
 */
#define DRV_RX_BUFFER_SIZE 100
#define DRV_TX_BUFFER_SIZE 100
/** \} */

/** \} */ // End group macro
// --------------------------------------------------------------- PUBLIC TYPES
/**
 * \defgroup type Types
 * \{
 */

/**
 * @brief Click ctx object definition.
 */
typedef struct
{
  // Output pins

  int rst;
  int cts;
  int dsr;

  // Input pins

  int dtr;
  int rts;

  // Modules

  int uart;

  char uart_rx_buffer[DRV_RX_BUFFER_SIZE];
  char uart_tx_buffer[DRV_TX_BUFFER_SIZE];

  uint8_t rsp_rdy;
  uint8_t termination_char;

} ble4_t;

/**
 * @brief Click configuration structure definition.
 */
typedef struct
{
  // Communication gpio pins

  int rx_pin;
  int tx_pin;

  // Additional gpio pins

  int dtr;
  int rst;
  int cts;
  int dsr;
  int rts;

  // static variable

  uint32_t baud_rate; // Clock speed.
  bool uart_blocking;
  UART_DataBits data_bit; // Data bits.
  UART_Parity parity_bit; // Parity bit.
  UART_StopBits stop_bit; // Stop bits.

} ble4_cfg_t;

/** \} */ // End types group
// ----------------------------------------------- PUBLIC FUNCTION DECLARATIONS

/**
 * \defgroup public_function Public function
 * \{
 */

  /**
 * @brief Initialization function.
 *
 * @param ctx Click object.
 * @param cfg Click configuration structure.
 * 
 * @description This function initializes all necessary pins and peripherals used for this click.
 */
  BLE4_RETVAL ble4_init(ble4_t *ctx);

  /**
 * @brief Reset function
 *
 * @param ctx  Click object.
 * 
 * @description This function allows user to reset a module.
 */
  void ble4_reset(ble4_t *ctx);

  /**
 * @brief Generic write function.
 * @param ble4 Click object.
 * @param data_buf Data buffer for sends.
 * @param len Number of bytes for sends.
 */
  void ble4_generic_write(ble4_t *ctx, char *data_buf, size_t len);

  /**
 * @brief Generic read function.
 * @param ble4 Click object.
 * @param data_buf Data buffer for read data.
 * @param max_len The maximum length of data that can be read.
 * @return Number of reads data.
 */
  size_t ble4_generic_read(ble4_t *ctx, char *data_buf, size_t max_len);

  /**
 * @brief Response Ready function
 *
 * @param ctx         Click object.  
 *
 * @returns 0 - response is not ready, 1 - response is ready
 *
 * @description This function checks does response ready or not.
 */
  uint8_t ble4_response_ready(ble4_t *ctx);

  /**
 * @brief Transmit function
 *
 * @param ctx         Click object.  
 * @param tx_data     Data to be transmitted
 * @param term_char   Command termination character
 *
 * @description This function allows user to transmit data and send commands to the module.
 *
 * @note This function will send a termination character ('\r' default) automatically at 
 * the end of the data transmitting.
 */
  void ble4_send_command(ble4_t *ctx, char *command, uint8_t term_char);

  /**
 * @brief Factory Reset command
 *
 * @param ctx         Click object. 
 * 
 * @description This command reset a module to factory defined defaults. 
 *
 * @note A reboot will be executed to update the new settings.
 */
  void ble4_fact_rst_cmd(ble4_t *ctx);

  /**
 * @brief Store Current Configuration command
 *
 * @param ctx         Click object. 
 *
 * @description This command commits all the settings to be stored in start up database.
 *
 * @note Executes +CPWROFF command to write the parameters to non-volatile memory.
 */
  void ble4_store_cnfg_cmd(ble4_t *ctx);

  /**
 * @brief Get Local Address command
 *
 * @description This command reads the local address of the bluetooth module.
 */
  void ble4_get_local_addr_cmd(ble4_t *ctx);

  /**
 * @brief Module Start Mode Setting command
 *
 * @param ctx         Click object.  
 * @param start_mode      0 - Command mode ( default )
 *                        1 - Data mode
 *                        2 - Extended data mode
 *                        3 - PPP mode
 *
 * @description This command sets the module start mode.
 */
  void ble4_set_start_mode_cmd(ble4_t *ctx, uint8_t start_mode);

  /**
 * @brief Get Module Start Mode command
 *
 * @param ctx         Click object. 
 * 
 * @description This command reads the module start mode.
 */
  void ble4_get_start_mode_cmd(ble4_t *ctx);

  /**
 * @brief Enter Data Mode command
 *
 * @param ctx         Click object.  
 * @param mode      0 - Command mode ( default )
 *                  1 - Data mode
 *                  2 - Extended data mode
 *                  3 - PPP mode
 *
 * @description This command requests the module to move to the new mode.
 * @note After executing the data mode command or the extended data mode command, 
 *       a delay of 50 ms is required before start of data transmission.
 */
  void ble4_enter_mode_cmd(ble4_t *ctx, uint8_t mode);

  /**
 * @brief Echo On/Off command
 *
 * @param ctx           Click object.  
 * @param echo_en       0 - Unit does not echo the characters in Command Mode
 *                      1 - Unit echoes the characters in Command Mode ( default )
 *
 * @description This command configures whether or not the unit echoes the characters received from the DTE in Command Mode.
 */
  void ble4_set_echo_cmd(ble4_t *ctx, uint8_t echo_en);

  /**
 * @brief Get Echo Setting command
 *
 * @param ctx           Click object.
 * 
 * @description This command reads current echo setting.
 */
  void ble4_get_echo_cmd(ble4_t *ctx);

  /**
 * @brief Local Name Setting command
 *
 * @param ctx           Click object.
 * @param local_name    Local name string to be set (maximum 29 characters)
 *
 * @description This command sets the local Bluetooth device name.
 *
 * @note 
 If user was enter a empty string, the local device name will be set to 
        "Bluetooth Device". For NINA-B31, the default name is "NINA-B31-XXXXXX", where XXXXXX is the last 6 characters from the Bluetooth address.
 */
  void ble4_set_local_name_cmd(ble4_t *ctx, char *local_name);

  /**
 * @brief Get Local Name command
 *
 * @param ctx           Click object.
 * 
 * @description This command reads the local Bluetooth device name.
 */
  void ble4_get_local_name_cmd(ble4_t *ctx);

  /**
 * @brief Bluetooth Low Energy Role Setting command
 *
 * @param ctx           Click object. 
 * @param le_role  
 * <pre> 
 *        0 - Disabled
 *        1 - Bluetooth low energy Central
 *        2 - Bluetooth low energy Peripheral ( default )
 *        3 - Bluetooth low energy Simultaneous Peripheral and Central
 * </pre>  
 *
 * @description This command sets the Bluetooth low energy role.
 * @note For the settings to take effect on NINA-B31, use the commands &W and +CPWROFF to store the configuration 
 *       to start up database and reboot the module.
 */
  void ble4_set_low_energy_role_cmd(ble4_t *ctx, uint8_t le_role);

  /**
 * @brief Get Bluetooth Low Energy Role command
 *
 * @param ctx           Click object.
 * 
 * @description This command reads the Bluetooth low energy role.
 */
  void ble4_get_low_energy_role_cmd(ble4_t *ctx);

  /**
 * @brief Get Peer List command
 *
 * @param ctx           Click object.
 * 
 * @description This command reads the connected peers (peer handle).
 *
 * @note The module will return a peer handle that identifies the connection, 
 *       protocol of the connection, local address and remote address if available.
 */
  void ble4_get_list_peers_cmd(ble4_t *ctx);

  /**
 * @brief Get Server Configuration command
 *
 * @param ctx           Click object.
 * 
 * @description This command reads server configuration.
 *
 * @note 
 * <pre>
 * The module will return a type of server:
 *       0 - Disabled, 1 - TCP, 2 - UDP, 3 - SPP, 4 - DUN, 5 - UUID, 
 *       6 - SPS (supported by NINA-B31), 7 - Reserved, 8 - ATP.
 * </pre>
 */
  void ble4_get_server_cnfg_cmd(ble4_t *ctx);

  /**
 * @brief Default Configuration command
 *
 * @param ctx           Click object.
 * 
 * @description This command resets the profile to the last stored configuration. 
 *
 * @note Any settings committed with AT&W will be discarded. 
 *       The restored settings will be used after a reboot, which this function also executes.
 */
  void ble4_set_default_cmd(ble4_t *ctx);

  /**
 * @brief Security Mode Setting command
 *
 * @param ctx           Click object. 
 * @param sec_mode  
 * <pre>
 *        1 - Security Disabled ( default )
 *        2 - Security Enabled - Just Works
 *        3 - Security Enabled - Display Only
 *        4 - Security Enabled - Display Yes/No
 *        5 - Security Enabled - Keyboard Only
 *        6 - Security Enabled - Out of band
 * </pre> 
 *
 * @description This command sets the security mode.
 *
 * @note 
 * <pre> For the settings to take effect on NINA-B31, use the commands &W and +CPWROFF to 
 *       store the configuration to start up database and reboot the module.
 *       NINA-B31 does not support Out of band and will fall back to Just Works pairing.
 * </pre>  
 */
  void ble4_set_sec_mode_cmd(ble4_t *ctx, uint8_t sec_mode);

  /**
 * @brief Get Security Mode command
 *
 * @param ctx           Click object.
 * 
 * @description This command reads the security mode.
 */
  void ble4_get_sec_mode_cmd(ble4_t *ctx);

  /**
 * @brief Pairing Mode Setting command
 *
 * @param ctx             Click object.
 * @param pairing_mode    1 - GAP non-pairing mode;   2 - GAP pairing mode ( default )
 *
 * @description This command sets the pairing mode.
 */
  void ble4_pairing_en_cmd(ble4_t *ctx, uint8_t pairing_mode);

  /**
 * @brief Get Pairing Mode command
 *
 * @param ctx           Click object.
 * 
 * @description This command reads the pairing mode.
 */
  void ble4_check_pairing_cmd(ble4_t *ctx);

  /**
 * @brief Connectability Mode Setting command
 *
 * @param ctx           Click object. 
 * @param conn_mode     1 - GAP non-connectable mode; 2 - GAP connectable mode ( default )
 *
 * @description This command sets the GAP connectability mode.
 */
  void ble4_connectability_en_cmd(ble4_t *ctx, uint8_t conn_mode);

  /**
 * @brief Get Connectability Mode command
 *
 * @param ctx           Click object.
 * 
 * @description This command reads the GAP connectability mode.
 */
  void ble4_check_connectability_cmd(ble4_t *ctx);

  /**
 * @brief Discoverability Mode Setting command
 *
 * @param ctx           Click object. 
 * @param discover_mode  
 * <pre> 
 *        1 - GAP non-discoverable mode
 *        2 - GAP limited discoverable mode
 *        3 - GAP general discoverable mode ( default )
 * </pre>  
 *
 * @description This command sets the GAP discoverability mode.
 *
 * @note For NINA-B31, the device will stay in the limited discoverable mode for 180 seconds.
 */
  void ble4_discoverability_en_cmd(ble4_t *ctx, uint8_t discover_mode);

  /**
 * @brief Get Discoverability Mode command
 *
 * @param ctx           Click object. 
 * 
 * @description This command reads the GAP discoverability mode.
 */
  void ble4_check_discoverability_cmd(ble4_t *ctx);

  /**
 * @brief Get Info command
 *
 * @param ctx           Click object. 
 * 
 * @description This command reads the all device and serial interface informations.
 */
  void ble4_get_info(ble4_t *ctx);

  /**
 * @brief SPS Pairing As Central Device command
 *
 * @param ctx          Click object.  
 * @param local_addr   Local address string of the target Bluetooth device ( peer ) 
 *                     [ 12 characters ]
 *
 * @description This command executes a SPS pairing as central device, sets data start mode 
 *              and reboot a module.
 *
 * @note This configuration will be stored to start up database.
 */
  uint8_t ble4_sps_central_pairing(ble4_t *ctx, uint8_t *local_addr);

  /**
 * @brief SPS Pairing As Peripheral Device command
 *
 * @param ctx          Click object.   
 *
 * @description This command allows SPS pairing as peripheral device, sets data start mode 
 *              and reboot a module.
 *
 * @note This configuration will be stored to start up database.
 */
  void ble4_sps_peripheral_pairing(ble4_t *ctx);

  /**
 * @brief CTS Pin Setting function
 *
 * @param ctx          Click object.   
 * @param state        0 - Low, 1 (or other value different from 0) - High
 *
 * @description This function sets the CTS pin to the desired state  
 *              ( UART clear to send control signal ).
 *
 * @note Used only when hardware flow control is enabled.
 *       The HW flow control is enabled by default.
 */
  void ble4_set_cts_pin(ble4_t *ctx, uint8_t state);

  /**
 * @brief Set DSR Pin function
 *
 * @param ctx          Click object.    
 * @param state        0 - Low, 1 ( or other value different from 0 ) - High
 *
 * @description This function sets the DSR pin to the desired state 
 *              ( UART data set ready signal ).
 *
 * @note Used to change system modes by default.
 */
  void ble4_set_dsr_pin(ble4_t *ctx, uint8_t state);

  /**
 * @brief Check DTR Pin function
 *
 * @param ctx          Click object. 
 *
 * @returns 0 or 1
 *
 * @description This function returns the state of the DTR pin
 *              ( UART data terminal ready signal).
 *
 * @note Used to indicate system status by default.
 */
  uint8_t ble4_get_dtr_pin(ble4_t *ctx);

  /**
 * @brief Check RTS Pin function
 *
 * @returns 0 or 1
 *
 * @description This function returns the state of the RTS pin 
 *              ( UART ready to send control signal ).
 *
 * @note Used only when hardware flow control is enabled.
 *       The HW flow control is enabled by default.
 */
  uint8_t ble4_get_rts_pin(ble4_t *ctx);

#endif // _BLE4_H_

/** \} */ // End public_function group
          /// \}    // End click Driver group
          /*! @} */
          // ------------------------------------------------------------------------- END