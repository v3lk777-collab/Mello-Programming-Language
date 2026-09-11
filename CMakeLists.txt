cmake_minimum_required(VERSION 3.25)

project(Mello VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(SOURCES
    src/main.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/ir/ir_generator.cpp
    src/compiler/compiler.cpp
    src/errors/error_handler.cpp
    src/semantic/symbol_table.cpp
    src/semantic/semantic_analyzer.cpp
    stdlib/module_loader.cpp
)

add_executable(mello ${SOURCES})

target_include_directories(mello PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ir
    ${CMAKE_CURRENT_SOURCE_DIR}/include/lexer
    ${CMAKE_CURRENT_SOURCE_DIR}/include/parser
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ast
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/include/errors
    ${CMAKE_CURRENT_SOURCE_DIR}/include/compiler
    ${CMAKE_CURRENT_SOURCE_DIR}/include/semantic
    ${CMAKE_CURRENT_SOURCE_DIR}/stdlib
)

if(MINGW)
    target_link_options(mello PRIVATE -static)
endif()