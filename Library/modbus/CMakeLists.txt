cmake_minimum_required(VERSION 3.13)
project(ModbusLibImporter LANGUAGES CXX)

# Create an imported static library target
# Create an imported static library target
add_library(modbus STATIC IMPORTED GLOBAL)

# Set the location of the static library file
# ${CMAKE_CURRENT_LIST_DIR} refers to the directory containing this CMakeLists.txt
set_target_properties(modbus PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libmodbus.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include"
)

# Create an alias so consumers can use target_link_libraries(my_app PRIVATE Modbus::modbus)
add_library(Modbus::modbus ALIAS modbus)

