cmake_minimum_required(VERSION 3.16)
project(Gallery VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt automoc, autorcc and autouic
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 libraries
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Include directories
include_directories(include)

# Add executable
add_executable(gallery
    src/main.cpp
    src/gallerywindow.h
    src/gallerywindow.cpp
    src/pff.cpp
    include/pff.h
)

# Link libraries
target_link_libraries(gallery PRIVATE Qt6::Widgets)
