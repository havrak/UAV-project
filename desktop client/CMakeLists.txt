cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME uav_desktop_control)
set(CMAKE_BUILD_TYPE Debug)
project(${PROJECT_NAME})

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(cpr REQUIRED)
find_package(osmgpsmap REQUIRED)
find_package(X11 REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package( Boost)
pkg_check_modules(GTKMM gtkmm-3.0)

link_directories(${GTKMM_LIBRARY_DIRS})

include_directories(${GTKMM_INCLUDE_DIRS})

include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 ")

add_library(inih STATIC
	"${PROJECT_SOURCE_DIR}/libraries/inih/cpp/INIReader.cpp"
	"${PROJECT_SOURCE_DIR}/libraries/inih/ini.c"
	)

set_target_properties(
	inih PROPERTIES
	COMPILE_FLAGS
	"-DINI_MAX_LINE=400"
)

target_include_directories(
	inih PUBLIC "${PROJECT_SOURCE_DIR}/libraries/libraries/inih/")

include_directories(include)
include_directories(${cpr_INCLUDE_DIR})
include_directories(${osmgpsmap_INCLUDE_DIR})

file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
	${OpenCV_LIBS}
	${GTKMM_LIBRARIES}
	${Boost_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${osmgpsmap_LIBRARY}
	X11
	inih
	cpr
	nlohmann_json
	)




