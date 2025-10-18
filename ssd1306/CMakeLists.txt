add_library(ssd1306 STATIC
    ssd1306.cpp
)

target_include_directories(ssd1306
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ssd1306
    PUBLIC
        pico_stdlib
        i2c
)
