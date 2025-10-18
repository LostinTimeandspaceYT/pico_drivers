# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(NAME my_project) # <-- change as needed.
set(PICO_BOARD pico CACHE STRING "Board type")

# Allow projects to point at a pre-built picotool to avoid network fetches.
set(PICO_PREFETCHED_PICOTOOL_PATH "" CACHE PATH
    "Optional path to an existing picotool install directory")
if(PICO_PREFETCHED_PICOTOOL_PATH STREQUAL "" AND DEFINED ENV{PICO_PREFETCHED_PICOTOOL_PATH})
    set(PICO_PREFETCHED_PICOTOOL_PATH $ENV{PICO_PREFETCHED_PICOTOOL_PATH})
endif()
if(PICO_PREFETCHED_PICOTOOL_PATH AND EXISTS "${PICO_PREFETCHED_PICOTOOL_PATH}/picotool/picotool")
    set(PICOTOOL_FETCH_FROM_GIT_PATH "${PICO_PREFETCHED_PICOTOOL_PATH}" CACHE PATH
        "Directory where picotool has been installed" FORCE)
elseif(PICO_PREFETCHED_PICOTOOL_PATH)
    message(WARNING "PICO_PREFETCHED_PICOTOOL_PATH='${PICO_PREFETCHED_PICOTOOL_PATH}' "
            "does not appear to contain a picotool installation (expected picotool/picotool). "
            "Falling back to Pico SDK default behaviour.")
endif()

# initialize the SDK based on PICO_SDK_PATH note: this must happen before
# project()
include(pico_sdk_import.cmake)

# boilerplate
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Main application sources
add_executable(${NAME} main.cpp)

# Include directories
add_subdirectory(./i2c)
add_subdirectory(./ap33772)
add_subdirectory(./stusb4500)
add_subdirectory(./ssd1306)
add_subdirectory(./ush)
add_subdirectory(./lcd/i2c_lcd)
add_subdirectory(./tps25750)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${NAME} 0)
pico_enable_stdio_usb(${NAME} 1)


target_link_libraries(${NAME}
    pico_stdlib
    i2c
    ap33772
    stusb4500
    ssd1306
    ush
    i2c_lcd
    tps25750
)

# create map/bin/hex/uf2 file in addition to ELF
pico_add_extra_outputs(${NAME})

# Set up files for the release packages
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
              ${CMAKE_CURRENT_LIST_DIR}/README.md DESTINATION .)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)
