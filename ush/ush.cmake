set(LIB_NAME ush)
add_library(${LIB_NAME} INTERFACE)

target_sources(${LIB_NAME} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/ush.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_autocomp.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_autocomp_state.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_autocomp_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_commands.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_file.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_node.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_node_mount.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_node_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_parse.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_parse_char.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_parse_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_process.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_prompt.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_read.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_read_char.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_read_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_write.c
    ${CMAKE_CURRENT_LIST_DIR}/ush_write_utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/ush_cmd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/ush_cmd_cd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/ush_cmd_help.c
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/ush_cmd_ls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/ush_cmd_pwd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/ush_cmd_cat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/ush_cmd_xxd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/ush_cmd_echo.c
)

target_include_directories(${LIB_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${LIB_NAME} INTERFACE 
    pico_stdlib
)