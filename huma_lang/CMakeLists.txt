cmake_minimum_required(VERSION 3.20)
project(huma_x VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /EHsc)
else()
    add_compile_options(-Wall -Wextra -pedantic -Wno-unused-parameter)
endif()

include_directories(include)

# Core HUMA X Library
set(HUMA_SOURCES
    src/common.cpp
    src/ast.cpp
    src/lexer.cpp
    src/parser.cpp
    src/semantic.cpp
    src/ir.cpp
    src/bytecode.cpp
    src/vm.cpp
    src/stdlib.cpp
    src/codegen_native.cpp
    src/codegen_wasm.cpp
    src/formatter.cpp
    src/repl.cpp
    src/docgen.cpp
    src/package_mgr.cpp
)

add_library(huma_core STATIC ${HUMA_SOURCES})
target_include_directories(huma_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# HUMA CLI Executable
add_executable(huma src/main.cpp)
target_link_libraries(huma PRIVATE huma_core)

# Tests
enable_testing()
add_executable(huma_test_suite tests/test_suite.cpp)
target_link_libraries(huma_test_suite PRIVATE huma_core)
add_test(NAME ComprehensiveTestSuite COMMAND huma_test_suite)

# Installation targets
install(TARGETS huma huma_core
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(DIRECTORY include/huma DESTINATION include)
