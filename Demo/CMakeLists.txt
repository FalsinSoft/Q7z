cmake_minimum_required(VERSION 3.14)

project(Q7zDemo LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_add_executable(${PROJECT_NAME}
    Main.cpp
    Q7zDemo.cpp
    Q7zDemo.qrc
    Q7zDemo.ui
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

#set(Q7Z_BUILD_STATIC 1)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Q7z)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Q7z Q7z)
target_link_libraries(${PROJECT_NAME} PRIVATE Q7z)
