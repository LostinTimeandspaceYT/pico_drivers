add_library(low_pass_filter STATIC
    low_pass_filter.cpp
)

target_include_directories(low_pass_filter
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(low_pass_filter
    PUBLIC
        pico_stdlib
)

add_library(pico_drivers::low_pass_filter ALIAS low_pass_filter)
