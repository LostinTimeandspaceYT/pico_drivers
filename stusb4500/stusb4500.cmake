set(LIB_NAME stusb4500)
add_library(${LIB_NAME} INTERFACE)

target_sources(${LIB_NAME} INTERFACE
${CMAKE_CURRENT_LIST_DIR}/${LIB_NAME}.cpp
)

target_include_directories(${LIB_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR})

# Pull in pico libraries that are needed
target_link_libraries(${LIB_NAME} INTERFACE 
pico_stdlib 
hardware_i2c
i2c 
)