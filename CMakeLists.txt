cmake_minimum_required(VERSION 3.20)
project(AxelGui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enforce Clang compiler
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "This project must be compiled using Clang. Run with: -DCMAKE_CXX_COMPILER=clang++")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(axel-gui
    src/main.cpp
    src/ThemeManager.h
    src/ThemeManager.cpp
    src/DownloadTask.h
    src/DownloadTask.cpp
    src/MainWindow.h
    src/MainWindow.cpp
)

target_link_libraries(axel-gui PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
