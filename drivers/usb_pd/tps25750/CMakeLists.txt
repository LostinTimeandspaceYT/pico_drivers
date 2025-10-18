add_library(tps25750 STATIC
    tps25750.cpp
)

target_include_directories(tps25750
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(tps25750
    PUBLIC
        pico_stdlib
        pico_drivers::i2c
)

add_library(pico_drivers::usb_pd::tps25750 ALIAS tps25750)
