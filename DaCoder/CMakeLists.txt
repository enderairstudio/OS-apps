cmake_minimum_required(VERSION 3.12)
project(DaCoder VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV (only core, imgcodecs, and imgproc components)
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs imgproc)

# Include directories
include_directories(include ${OpenCV_INCLUDE_DIRS})

# Library sources
set(LIB_SOURCES
    src/pff.cpp
)

# Executable sources
set(EXE_SOURCES
    src/main.cpp
)

# Create shared library for pff format so it can be reused
add_library(pff SHARED ${LIB_SOURCES})
target_include_directories(pff PUBLIC include)

# Create CLI executable
add_executable(dacoder ${EXE_SOURCES})
target_link_libraries(dacoder PRIVATE pff ${OpenCV_LIBS})

# Print configuration info
message(STATUS "OpenCV library status:")
message(STATUS "  version: ${OpenCV_VERSION}")
message(STATUS "  libraries: ${OpenCV_LIBS}")
message(STATUS "  include path: ${OpenCV_INCLUDE_DIRS}")
