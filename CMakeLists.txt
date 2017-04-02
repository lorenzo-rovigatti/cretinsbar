cmake_minimum_required(VERSION 2.8)
project(cretinsbar)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CRETINSBAR_VERSION 0)

option(G "Set to ON to compile with optimisations and debug symbols" OFF)

if(G)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
	message(STATUS "Compiling in Release+Debug symbols mode")
endif(G)

if(CMAKE_VERSION VERSION_LESS "3.1")
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_CXX_FLAGS}")
	endif()
else()
	set(CMAKE_CXX_STANDARD 11)
endif()

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-DQT_MESSAGELOGCONTEXT -Wall -DCRETINSBAR_VERSION=${CRETINSBAR_VERSION}) 

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(SoundTouch REQUIRED)

# The Qt5Widgets_INCLUDES also includes the include directories for dependencies QtCore and QtGui
include_directories(${Qt5Widgets_INCLUDES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/GUI ${CMAKE_CURRENT_SOURCE_DIR}/src/GUI/qcustomplot)

# Boost's headers
include_directories(${Boost_INCLUDE_DIRS})

set(QT5_LIBRARIES Qt5::Widgets Qt5::Multimedia Qt5::PrintSupport)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(SOURCES
	src/main.cpp
	src/CretinsBar.cpp
	src/Engine.cpp
	src/SoundUtils/SoundUtils.cpp
	src/SoundUtils/Wave.cpp
	src/GUI/MainWindow.cpp
	src/GUI/WaveForm.cpp
	src/GUI/qcustomplot/qcustomplot.cpp
)

set(UI_SOURCES
	src/GUI/MainWindow.ui
)

set(UI_RESOURCES

)

qt5_wrap_ui(UI_GENERATED_HEADERS ${UI_SOURCES})
qt5_add_resources(UI_GENERATED_RESOURCES ${UI_RESOURCES})

add_executable(cretinsbar ${SOURCES} ${UI_GENERATED_HEADERS} ${UI_GENERATED_RESOURCES})

target_link_libraries(cretinsbar ${QT5_LIBRARIES} ${SOUNDTOUCH_LIBRARIES})
