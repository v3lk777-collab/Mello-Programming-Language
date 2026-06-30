cmake_minimum_required(VERSION 3.12)
project(Mello VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    include
    include/ast
    include/lexer
    include/parser
)

set(SOURCES
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
)

add_executable(mello ${SOURCES})

target_include_directories(mello PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)