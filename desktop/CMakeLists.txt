cmake_minimum_required(VERSION 3.16)
project(desktop VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets SerialPort Core)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

qt_add_executable(desktop
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(desktop PRIVATE Qt6::Widgets Qt6::SerialPort Qt6::Core)
qt_finalize_executable(desktop)
