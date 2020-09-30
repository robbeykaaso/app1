cmake_minimum_required(VERSION 3.5)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#if (CMAKE_BUILD_TYPE STREQUAL Debug)
#set(${CMAKE_BINARY_DIR} install)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/bin)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY D:/deepinspection/install/${CMAKE_BUILD_TYPE}/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY D:/deepinspection/install/${CMAKE_BUILD_TYPE}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY D:/deepinspection/install/${CMAKE_BUILD_TYPE}/bin)

project(WINPACK)

find_package(Qt5 COMPONENTS Core REQUIRED)

# 创建可执行程序

add_executable(${PROJECT_NAME}
    "main.cpp")

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core )
#add_executable(hello_binary
#src/main.cpp
#)

# 链接动态库文件
#target_link_libraries( hello_binary PRIVATE hello::library)
