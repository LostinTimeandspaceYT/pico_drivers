add_library(stusb4500 STATIC
    stusb4500.cpp
)

target_include_directories(stusb4500
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(stusb4500
    PUBLIC
        pico_stdlib
        pico_drivers::i2c
)

add_library(pico_drivers::usb_pd::stusb4500 ALIAS stusb4500)
