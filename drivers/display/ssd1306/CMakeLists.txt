add_library(ssd1306 STATIC
    ssd1306.cpp
)

target_include_directories(ssd1306
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(ssd1306
    PUBLIC
        pico_stdlib
        pico_drivers::i2c
        hardware_dma
        hardware_spi
        hardware_gpio
)

add_library(pico_drivers::display::ssd1306 ALIAS ssd1306)
