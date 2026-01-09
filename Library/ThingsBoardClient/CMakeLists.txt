cmake_minimum_required(VERSION 3.13)
project(ThingsBoardClientImporter LANGUAGES CXX)

# Create an imported static library target
# Create an imported static library target
add_library(thingsboard_client STATIC IMPORTED GLOBAL)

# Set the location of the static library file
set_target_properties(thingsboard_client PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/libthingsboard_client.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/include"
)

# Create an alias so consumers can use target_link_libraries(my_app PRIVATE ThingsBoard::client)
add_library(ThingsBoard::client ALIAS thingsboard_client)
