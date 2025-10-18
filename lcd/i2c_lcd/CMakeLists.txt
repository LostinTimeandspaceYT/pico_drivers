add_library(i2c_lcd STATIC
    i2c_lcd.cpp
)

target_include_directories(i2c_lcd
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(i2c_lcd
    PUBLIC
        pico_stdlib
        i2c
)
