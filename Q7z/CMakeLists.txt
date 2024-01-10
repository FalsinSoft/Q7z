cmake_minimum_required(VERSION 3.14)

project(Q7z LANGUAGES C CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

file(GLOB_RECURSE SOURCES "*.h" "*.cpp" "*.c")
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE Q7Z_LIB)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core)
