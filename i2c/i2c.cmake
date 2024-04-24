set(LIB_NAME i2c)
add_library(${LIB_NAME} INTERFACE)

target_sources(${LIB_NAME} INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/${LIB_NAME}.cpp
)

target_link_libraries(${LIB_NAME} INTERFACE 
pico_stdlib
hardware_i2c
)