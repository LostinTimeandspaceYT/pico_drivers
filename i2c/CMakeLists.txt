add_library(i2c STATIC
    i2c.cpp
)

target_include_directories(i2c
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(i2c
    PUBLIC
        pico_drivers::utils
        pico_stdlib
        hardware_i2c
)

add_library(pico_drivers::i2c ALIAS i2c)
