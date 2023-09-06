cmake_minimum_required(VERSION 3.22)
project(mqtt_temp)

add_executable(${PROJECT_NAME} main.cpp)

find_package(Qt5 COMPONENTS Network REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
target_link_libraries(${PROJECT_NAME} Qt5::Network)
target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} mosquittopp)