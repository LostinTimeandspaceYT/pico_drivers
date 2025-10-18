add_library(i2c STATIC
    i2c.cpp
)

target_include_directories(i2c
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(i2c
    PUBLIC
        pico_stdlib
        hardware_i2c
)
