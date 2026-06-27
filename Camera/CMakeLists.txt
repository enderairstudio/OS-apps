cmake_minimum_required(VERSION 3.16)
project(Camera VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt automoc, autorcc and autouic
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 libraries
find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)

# Include directories
include_directories(include)

# Add executable
add_executable(camera
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/cameraview.h
    src/cameraview.cpp
    src/settingsmenu.h
    src/settingsmenu.cpp
    src/pff.cpp
    include/pff.h
)

# Link libraries
target_link_libraries(camera PRIVATE Qt6::Widgets Qt6::Multimedia)
