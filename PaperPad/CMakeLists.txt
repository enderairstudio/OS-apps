cmake_minimum_required(VERSION 3.15)
project(PaperPad CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/MainWindow.cpp
    src/core/EditorCtrl.cpp
    src/core/SidebarCtrl.cpp
    src/core/MenuBar.cpp
    src/core/FindDialog.cpp
    src/io/FileHandler.cpp
    src/utils/Logger.cpp
)

# Header files
set(HEADERS
    include/core/Application.hpp
    include/core/MainWindow.hpp
    include/core/EditorCtrl.hpp
    include/core/SidebarCtrl.hpp
    include/core/MenuBar.hpp
    include/core/FindDialog.hpp
    include/io/FileHandler.hpp
    include/utils/Logger.hpp
)

add_executable(PaperPad WIN32 ${SOURCES} ${HEADERS})

target_include_directories(PaperPad PUBLIC include)

# Add linking flags for Windows APIs and Win32 GUI mode (-mwindows)
if(WIN32)
    target_link_libraries(PaperPad PRIVATE 
        comctl32 
        comdlg32 
        user32 
        gdi32 
        shell32 
        shlwapi
    )
    # Link with -mwindows to hide the console window when executing
    target_link_options(PaperPad PRIVATE -mwindows)
endif()
