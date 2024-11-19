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
set(PICO_BOARD pico2 CACHE STRING "Board type")

# initialize the SDK based on PICO_SDK_PATH note: this must happen before
# project()
include(pico_sdk_import.cmake)

# boilerplate
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Add source files here
add_executable(${NAME} main.cpp # <-- add source files here!
  ush/ush.c
  ush/ush_read.c
  ush/ush_read_utils.c
  ush/ush_read_char.c
  ush/ush_parse.c
  ush/ush_parse_char.c
  ush/ush_parse_utils.c
  ush/ush_write.c
  ush/ush_write_utils.c
  ush/ush_prompt.c
  ush/ush_reset.c
  ush/ush_file.c
  ush/ush_node.c
  ush/ush_node_utils.c
  ush/ush_node_mount.c
  ush/ush_utils.c
  ush/ush_commands.c
  ush/ush_process.c
  ush/ush_autocomp.c
  ush/ush_autocomp_utils.c
  ush/ush_autocomp_state.c
  ush/commands/ush_cmd.c
  ush/commands/ush_cmd_cd.c
  ush/commands/ush_cmd_help.c
  ush/commands/ush_cmd_ls.c
  ush/commands/ush_cmd_pwd.c
  ush/commands/ush_cmd_cat.c
  ush/commands/ush_cmd_xxd.c
  ush/commands/ush_cmd_echo.c
  ush/picoshell.cpp
  ush/picoshell_cmd.cpp
  ush/node_root.cpp
  ush/node_bin.cpp
  ush/node_dev.cpp
  ush/node_i2c.cpp
)

# Include directories
add_subdirectory(./i2c)
add_subdirectory(./ap33772)
add_subdirectory(./stusb4500)
add_subdirectory(./ssd1306)
add_subdirectory(./ush)

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
)

# create map/bin/hex/uf2 file in addition to ELF
pico_add_extra_outputs(${NAME})

# Set up files for the release packages
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
              ${CMAKE_CURRENT_LIST_DIR}/README.md DESTINATION .)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)
