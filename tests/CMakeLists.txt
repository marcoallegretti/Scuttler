# SPDX-FileCopyrightText: 2026 Marco Allegretti
# SPDX-License-Identifier: EUPL-1.2

find_package(Qt6 REQUIRED COMPONENTS QmlTools Test)

qt_add_executable(tst_agama tst_agama.cpp)
target_link_libraries(tst_agama PRIVATE scuttler_agama Qt6::Test)
add_test(NAME agama COMMAND tst_agama)

qt_add_executable(tst_cli tst_cli.cpp)
target_compile_definitions(tst_cli PRIVATE
    SCUTTLER_BINARY="$<TARGET_FILE:scuttler>"
    SCUTTLER_VERSION="${PROJECT_VERSION}"
)
target_link_libraries(tst_cli PRIVATE Qt6::Test)
add_dependencies(tst_cli scuttler)
add_test(NAME cli COMMAND tst_cli)
set_tests_properties(cli PROPERTIES TIMEOUT 60)

add_test(NAME qmllint COMMAND Qt6::qmllint --max-warnings 0 ${PROJECT_SOURCE_DIR}/main.qml)

option(SCUTTLER_WAYLAND_TESTS "Run Scuttler under a headless cage compositor" OFF)
if(SCUTTLER_WAYLAND_TESTS)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    find_program(CAGE_EXECUTABLE cage REQUIRED)
    add_test(NAME wayland-smoke
        COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/wayland_smoke.py
            $<TARGET_FILE:scuttler> ${CAGE_EXECUTABLE})
    set_tests_properties(wayland-smoke PROPERTIES TIMEOUT 180)
endif()
