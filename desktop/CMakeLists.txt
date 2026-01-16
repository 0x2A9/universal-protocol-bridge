cmake_minimum_required(VERSION 3.16)
project(desktop VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets SerialPort)

set(DCP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../dcp")

include_directories("${DCP_ROOT}/inc")

set(DCP_SOURCES
    "${DCP_ROOT}/src/protocol.c"
    "${DCP_ROOT}/src/tlv.c"
)

include_directories(../dcp/inc)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

qt_add_executable(desktop
    MANUAL_FINALIZATION
    ${DCP_SOURCES}
    ${PROJECT_SOURCES}
    protocolmanager.h
    protocolmanager.cpp
)

target_link_libraries(desktop PRIVATE Qt6::Widgets Qt6::SerialPort)
qt_finalize_executable(desktop)
