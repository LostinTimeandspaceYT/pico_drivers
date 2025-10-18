add_library(tps25750 STATIC
    tps25750.cpp
)

target_include_directories(tps25750
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(tps25750
    PUBLIC
        pico_stdlib
        i2c
)
