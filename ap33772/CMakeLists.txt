add_library(ap33772 STATIC
    ap33772.cpp
)

target_include_directories(ap33772
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ap33772
    PUBLIC
        pico_stdlib
        i2c
)
