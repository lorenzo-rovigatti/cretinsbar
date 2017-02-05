cmake_minimum_required(VERSION 2.8)
project(cretinsbar)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CRETINSBAR_VERSION 0)

OPTION(G "Set to ON to compile with optimisations and debug symbols" OFF)

IF(G)
	SET(CMAKE_BUILD_TYPE RelWithDebInfo)
	MESSAGE(STATUS "Compiling in Release+Debug symbols mode")
ENDIF(G)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-DQT_MESSAGELOGCONTEXT -Wall -DCRETINSBAR_VERSION=${CRETINSBAR_VERSION}) 

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_package(SoundTouch)

# The Qt5Widgets_INCLUDES also includes the include directories for dependencies QtCore and QtGui
include_directories(${Qt5Widgets_INCLUDES})

# This allows ui_MainWindow.h to find qcustomplot.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/qcustomplot)

# Boost's headers
include_directories(${Boost_INCLUDE_DIRS})

set(QT5_LIBRARIES Qt5::Widgets Qt5::Multimedia)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(SOURCES
	src/main.cpp
	src/CretinsBar.cpp
	src/SoundUtils/SoundUtils.cpp
	src/Utilities/WavAnalyser.cpp
	src/GUI/MainWindow.cpp
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
