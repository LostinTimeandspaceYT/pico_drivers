add_library(ap33772 STATIC
    ap33772.cpp
)

target_include_directories(ap33772
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(ap33772
    PUBLIC
        pico_stdlib
        pico_drivers::i2c
)

add_library(pico_drivers::usb_pd::ap33772 ALIAS ap33772)
