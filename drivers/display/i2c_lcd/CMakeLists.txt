add_library(i2c_lcd STATIC
    i2c_lcd.cpp
)

target_include_directories(i2c_lcd
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(i2c_lcd
    PUBLIC
        pico_stdlib
        pico_drivers::utils
        pico_drivers::i2c
)

add_library(pico_drivers::display::i2c_lcd ALIAS i2c_lcd)
