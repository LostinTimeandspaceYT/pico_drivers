add_library(stusb4500 STATIC
    stusb4500.cpp
)

target_include_directories(stusb4500
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(stusb4500
    PUBLIC
        pico_stdlib
        hardware_i2c
        i2c
)
