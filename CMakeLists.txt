cmake_minimum_required(VERSION 3.16)

project(modbus_tb LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Main executable
add_executable(main main.cpp src/iPM2xxx.cpp src/iA9MEM15.cpp src/energy_calc.cpp)

# 2. Include directories
set(USER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(main PRIVATE
    ${USER_INCLUDE_DIR}
)

# 3. SQLite3
add_library(sqlite3 STATIC Library/sqlite3/sqlite3.c)
target_include_directories(sqlite3 PUBLIC Library/sqlite3)

# 4. Subdirectories
add_subdirectory(Library/modbus)
add_subdirectory(Library/ThingsBoardClient)

# 5. OpenSSL
find_package(OpenSSL REQUIRED)
if(NOT OpenSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found")
endif()

# 6. Paho MQTT C (SSL)
find_path(PAHO_MQTT_INCLUDE_DIR
    NAMES MQTTClient.h MQTTAsync.h
    PATHS /usr/local/include /usr/include
)

find_library(PAHO_MQTT_LIB
    NAMES paho-mqtt3cs paho-mqtt3c
    PATHS /usr/local/lib /usr/lib
)

if(NOT PAHO_MQTT_INCLUDE_DIR OR NOT PAHO_MQTT_LIB)
    message(FATAL_ERROR "Paho MQTT C (SSL) not found")
endif()

target_include_directories(main PRIVATE
    ${PAHO_MQTT_INCLUDE_DIR}
)

# 7. Link libraries (ลิงก์ OpenSSL หลังทุกไลบรารีที่ต้องใช้)
target_link_libraries(main PRIVATE
    Modbus::modbus
    ThingsBoard::client
    sqlite3
    ${PAHO_MQTT_LIB}
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
)
