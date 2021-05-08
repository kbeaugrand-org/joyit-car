#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

cmake_minimum_required (VERSION 3.10)

project (JoyItCar C)

azsphere_configure_tools(TOOLS_REVISION "21.04")
azsphere_configure_api(TARGET_API_SET "9")

# Create executable
add_executable (${PROJECT_NAME} 
                    main.c 
                    eventloop_timer_utilities.c 
                    utils.c 
                    azure_iot_client.c 
                    i2c_motor_driver.c 
                    button_behavior.c
                    libs/ble4_click/ble4.c
                    ble_commands.c)

target_include_directories(${PROJECT_NAME} PUBLIC 
                        ${AZURE_SPHERE_API_SET_DIR}/usr/include/azureiot
                        ${AZURE_SPHERE_API_SET_DIR}/usr/include/azure_prov_client 
                        ${AZURE_SPHERE_API_SET_DIR}/usr/include/azure_c_shared_utility)

target_compile_definitions(${PROJECT_NAME} PUBLIC AZURE_IOT_HUB_CONFIGURED)

target_link_libraries (${PROJECT_NAME} applibs pthread gcc_s c azureiot)

azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "./HardwareDefinitions/avnet_mt3620_sk" 
                                                    TARGET_DEFINITION "joyitcar_appliance.json")

azsphere_target_add_image_package(${PROJECT_NAME})