set(LIB_NAME ush)

file(GLOB_RECURSE USH_SRC_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(${LIB_NAME} ${USH_SRC_FILES})

target_include_directories(${LIB_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${LIB_NAME}
  PUBLIC
    pico_stdlib
    hardware_i2c
    tps25750
)
