cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME uav_drone)
project(${PROJECT_NAME})

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(wiringPi REQUIRED)
find_package(bcm2835 REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

add_library(
  cjy901 STATIC
  "${PROJECT_SOURCE_DIR}/libraries/Raspberry-JY901-Serial/JY901_Serial.cpp")
target_include_directories(
  cjy901 PUBLIC "${PROJECT_SOURCE_DIR}/libraries/Raspberry-JY901-Serial/")

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories(${WIRINGPI_INCLUDE_DIRS})
include_directories(${BCM2835_INCLUDE_DIRS})

file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

# target_link_libraries( ${PROJECT_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT}
# ${WIRINGPI_LIBRARIES} cjy901
# "${PROJECT_SOURCE_DIR}/libraries/rpidmx512-Lib-PCA9685/libpca9685.a")

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC ${CMAKE_THREAD_LIBS_INIT} ${WIRINGPI_LIBRARIES} ${BCM2835_LIBRARIES}
         cjy901
         "${PROJECT_SOURCE_DIR}/libraries/rpidmx512-Lib-PCA9685/libpca9685.a")

# target_link_libraries( ${PROJECT_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT}
# ${WIRINGPI_LIBRARIES} bcm2835 cjy901
# "${PROJECT_SOURCE_DIR}/libraries/rpidmx512-Lib-PCA9685/libpca9685.a")
