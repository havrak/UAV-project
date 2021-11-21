cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME uav_drone)
project(${PROJECT_NAME})

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(wiringPi REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

add_library(cjy901 STATIC "${PROJECT_SOURCE_DIR}/libraries/Raspberry-JY901-Serial/JY901_Serial.cpp")
target_include_directories(cjy901 PUBLIC "${PROJECT_SOURCE_DIR}/libraries/Raspberry-JY901-Serial/")

add_library(pca9685 STATIC "${PROJECT_SOURCE_DIR}/libraries/lib-pca9685/pca9685.cpp" "${PROJECT_SOURCE_DIR}/libraries/lib-pca9685/pca9685pwmled.cpp" "${PROJECT_SOURCE_DIR}/libraries/lib-pca9685/pca9685servo.cpp")
target_include_directories(pca9685 PUBLIC "${PROJECT_SOURCE_DIR}/libraries/lib-pca9685/")


include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories(${WIRINGPI_INCLUDE_DIRS})

file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}  PUBLIC ${CMAKE_THREAD_LIBS_INIT} ${WIRINGPI_LIBRARIES} cjy901 pca9685)

