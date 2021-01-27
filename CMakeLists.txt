cmake_minimum_required(VERSION 3.1)

project(${CMAKE_CUSTOM_PROJECT} LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_compile_options(-bigobj)
#ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

#set(deepinspectapptemplate_ROOT)
#get_filename_component(deepinspectapptemplate_ROOT ${PROJECT_SOURCE_DIR} DIRECTORY)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

#message("include: ${PROJECT_SOURCE_DIR}")
find_package(Qt5 COMPONENTS Widgets Core Quick REQUIRED)
set(REA_DIR ${CMAKE_CURRENT_LIST_DIR}/../frm/cmake)
#message("xxx: ${CMAKE_CURRENT_LIST_DIR}")
find_package(REA)
#find_package(PythonLibs REQUIRED)

#qtquick_compiler_add_resources(RESOURCES qml.qrc)
#find_package(VTK REQUIRED)

set(my_Sources
    "main.cpp" #WIN32: to disable the console window
    "${PROJECT_NAME}/resource/logo.rc"
    )

set(my_Libs
    Qt5::Core Qt5::Quick Qt5::Widgets
    ${REA_LIBS}
    dbghelp
    )

add_executable(${PROJECT_NAME}
    ${my_Sources}
    )
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${my_Libs}
    )

include_directories(${REA_INCLUDE_DIRS})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/../frm/install/bin/${CMAKE_BUILD_TYPE}"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "copying dll.")

#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#    "${CMAKE_SOURCE_DIR}/../frm/qml"
#    $<TARGET_FILE_DIR:${PROJECT_NAME}>
#    COMMENT "copying gui.")
