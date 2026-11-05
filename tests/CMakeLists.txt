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
