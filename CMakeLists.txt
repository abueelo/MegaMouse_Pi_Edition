# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(MegaMouse_Pi_Edition C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(MegaMouse_Pi_Edition
        main.cpp
        Motors.cpp
        SensorArray.cpp
        Drivetrain.cpp
        Maze.cpp
        MazeSolver.cpp
        Logger.cpp
        )

pico_set_program_name(MegaMouse_Pi_Edition "MegaMouse_Pi_Edition")
pico_set_program_version(MegaMouse_Pi_Edition "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(MegaMouse_Pi_Edition 0)
pico_enable_stdio_usb(MegaMouse_Pi_Edition 1)

# Add the standard library to the build
target_link_libraries(MegaMouse_Pi_Edition
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(MegaMouse_Pi_Edition PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/Ble
)

pico_btstack_make_gatt_header(MegaMouse_Pi_Edition PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Ble/MegaMouseBle.gatt)

# Add any user requested libraries
target_link_libraries(MegaMouse_Pi_Edition
        pico_cyw43_arch_none
        hardware_pwm
        pico_btstack_ble
        pico_btstack_cyw43
        )

pico_add_extra_outputs(MegaMouse_Pi_Edition)

