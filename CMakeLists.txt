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

project(my_project LANGUAGES C CXX ASM) # <-- change the project name here as needed.

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Main application sources
add_executable(${PROJECT_NAME} main.cpp)

add_library(pico_drivers_utils INTERFACE)
add_library(pico_drivers::utils ALIAS pico_drivers_utils)
target_include_directories(pico_drivers_utils
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# List driver libraries and their directory relative paths as <lib>:<subdir>
set(DRIVER_MODULES
    "i2c:i2c"
    # "ap33772:drivers/usb_pd/ap33772"
    # "stusb4500:drivers/usb_pd/stusb4500"
    "ssd1306:drivers/display/ssd1306"
    "low_pass_filter:drivers/low_pass_filter"
    # "i2c_lcd:drivers/display/i2c_lcd"
    # "tps25750:drivers/usb_pd/tps25750"
)

set(DRIVER_LIBS)

foreach(module ${DRIVER_MODULES})
    string(REPLACE ":" ";" module_parts "${module}")
    list(GET module_parts 0 DRIVER_LIB)
    list(GET module_parts 1 DRIVER_SUBDIR)
    add_subdirectory(${DRIVER_SUBDIR})
    list(APPEND DRIVER_LIBS ${DRIVER_LIB})
endforeach()

add_subdirectory(ush)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)


target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_i2c
    ${DRIVER_LIBS}
    ush
)

# create map/bin/hex/uf2 file in addition to ELF
pico_add_extra_outputs(${PROJECT_NAME})

# Set up files for the release packages
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.uf2
              ${CMAKE_CURRENT_LIST_DIR}/README.md DESTINATION .)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)
