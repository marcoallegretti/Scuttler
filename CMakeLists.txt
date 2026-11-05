# SPDX-FileCopyrightText: 2026 Marco Allegretti
# SPDX-License-Identifier: EUPL-1.2

cmake_minimum_required(VERSION 3.21)
project(scuttler VERSION 0.1.0 LANGUAGES CXX)

option(BUILD_TESTING "Build the tests" ON)

include(CheckPIESupported)
include(GNUInstallDirs)
check_pie_supported()
set(CMAKE_AUTOMOC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Qt6 6.11 REQUIRED COMPONENTS Core Gui Qml Quick WebEngineQuick)

add_library(scuttler_agama STATIC src/agama.cpp)
target_include_directories(scuttler_agama PUBLIC src)
target_link_libraries(scuttler_agama PUBLIC Qt6::Core)

qt_add_executable(scuttler main.cpp)
qt_add_resources(scuttler qml FILES main.qml)
target_compile_definitions(scuttler PRIVATE SCUTTLER_VERSION="${PROJECT_VERSION}")
target_link_libraries(scuttler PRIVATE
    scuttler_agama
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::WebEngineQuick
)

install(TARGETS scuttler)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
