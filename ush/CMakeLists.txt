set(USH_SOURCES
  commands/ush_cmd.c
  commands/ush_cmd_cat.c
  commands/ush_cmd_cd.c
  commands/ush_cmd_echo.c
  commands/ush_cmd_help.c
  commands/ush_cmd_ls.c
  commands/ush_cmd_pwd.c
  commands/ush_cmd_xxd.c
  node_bin.cpp
  node_dev.cpp
  node_root.cpp
  picoshell.cpp
  picoshell_cmd.cpp
  ush.c
  ush_autocomp.c
  ush_autocomp_state.c
  ush_autocomp_utils.c
  ush_commands.c
  ush_file.c
  ush_node.c
  ush_node_mount.c
  ush_node_utils.c
  ush_parse.c
  ush_parse_char.c
  ush_parse_utils.c
  ush_process.c
  ush_prompt.c
  ush_read.c
  ush_read_char.c
  ush_read_utils.c
  ush_reset.c
  ush_utils.c
  ush_write.c
  ush_write_utils.c
)

set(USH_LINK_LIBS
  pico_stdlib
  pico_drivers::utils
)

set(USH_DEFINES)

if("i2c" IN_LIST DRIVER_LIBS)
  list(APPEND USH_SOURCES node_i2c.cpp)
  list(APPEND USH_LINK_LIBS hardware_i2c pico_drivers::i2c)
  list(APPEND USH_DEFINES USH_ENABLE_NODE_I2C)
endif()

if("tps25750" IN_LIST DRIVER_LIBS)
  list(APPEND USH_SOURCES node_tps25750.cpp)
  list(APPEND USH_LINK_LIBS pico_drivers::usb_pd::tps25750)
  list(APPEND USH_DEFINES USH_ENABLE_NODE_TPS25750)
endif()

add_library(ush ${USH_SOURCES})

target_include_directories(ush
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ush
  PUBLIC
    ${USH_LINK_LIBS}
)

if(USH_DEFINES)
  target_compile_definitions(ush
    PUBLIC
      ${USH_DEFINES}
  )
endif()
