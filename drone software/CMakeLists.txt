cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

set(PROJECT_NAME uav_drone)

project(${PROJECT_NAME})

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(wiringPi REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

add_library(
	cjy901 STATIC
	"${PROJECT_SOURCE_DIR}/libraries/Raspberry-JY901-Serial/JY901.cpp")
target_include_directories(
	cjy901 PUBLIC "${PROJECT_SOURCE_DIR}/libraries/Raspberry-JY901-Serial/")

add_library(ina226 STATIC
	"${PROJECT_SOURCE_DIR}/libraries/raspberry-pi-ina226/ina226.cpp")
target_include_directories(
	ina226 PUBLIC "${PROJECT_SOURCE_DIR}/libraries/raspberry-pi-ina226/")

add_library(pca9685 STATIC
	"${PROJECT_SOURCE_DIR}/libraries/lib-pca9685/pca9685.cpp"
	"${PROJECT_SOURCE_DIR}/libraries/lib-pca9685/pca9685servo.cpp"
	)
target_include_directories(
	pca9685 PUBLIC "${PROJECT_SOURCE_DIR}/libraries/lib-pca9685/")

add_library(inih STATIC
	"${PROJECT_SOURCE_DIR}/libraries/inih/INIReader.cpp")
target_include_directories(
	inih PUBLIC "${PROJECT_SOURCE_DIR}/libraries/libraries/inih/")

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories(${WIRINGPI_INCLUDE_DIRS})

file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(
	${PROJECT_NAME}
	PUBLIC ${CMAKE_THREAD_LIBS_INIT}
	${WIRINGPI_LIBRARIES}
	${OpenCV_LIBS}
	cjy901
	ina226
	pca9685
	)
