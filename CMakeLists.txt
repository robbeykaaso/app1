cmake_minimum_required(VERSION 3.1)

project(DeepInspection LANGUAGES CXX)

add_definitions( -DRX_USE_EXCEPTIONS=OFF )
set(USEAWS ON)

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
find_package(Qt5 COMPONENTS Widgets Core Quick Sql SerialPort SerialBus REQUIRED)
find_package(OpenCV REQUIRED)
set(REA_DIR ${CMAKE_CURRENT_LIST_DIR}/../frm/cmake)
#message("xxx: ${CMAKE_CURRENT_LIST_DIR}")
find_package(REA)
#find_package(PythonLibs REQUIRED)

#find_package(DeepInspectSDK REQUIRED)
#find_package(RapidJSON REQUIRED)

#qtquick_compiler_add_resources(RESOURCES qml.qrc)
#find_package(VTK REQUIRED)

set(my_Sources
    "main.cpp" #WIN32: to disable the console window
    "storage/storage.h"
    "storage/storage.cpp"
    "storage/mysql.cpp"
    #"device/camera.cpp"
    #"device/io.cpp"
    "util/cv.h"
    "util/cv.cpp"
    "util/pack.cpp"
#    "util/py.h"
#    "util/py.cpp"
    "deepsight_anno/storage.h"
    "deepsight_anno/storage.cpp"
    "deepsight_anno/protocal.h"
    "deepsight_anno/model.h"
    "deepsight_anno/model.cpp"
    "deepsight_anno/user.cpp"
    "deepsight_anno/project.cpp"
    "deepsight_anno/task.cpp"
    "deepsight_anno/task.h"
    "deepsight_anno/custom.cpp"
    "qml.qrc"
    "resource/logo.rc"
    )

set(my_Libs
    Qt5::Core Qt5::Quick Qt5::Widgets Qt5::Sql Qt5::SerialPort Qt5::SerialBus
    ${REA_LIBS}
    ${OpenCV_LIBS}
    )

if (${USEAWS} STREQUAL "ON")
    message("use aws")
    find_package(AWSSDK REQUIRED COMPONENTS core s3)
    add_definitions(-DUSEAWS)
    add_executable(${PROJECT_NAME}
        ${my_Sources}
        "storage/aws_s3.h"
        "storage/aws_s3.cpp"
        "storage/awsStorage.h"
        "storage/awsStorage.cpp"
        )
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${my_Libs}
        ${AWSSDK_LINK_LIBRARIES}

    #    ${DeepInspectSDK_LIBS}
    #    ${PYTHON_LIBRARIES}
        )# ${VTK_LIBRARIES})# #  ${DeepInspectSDK_LIBS} ${gflags_LIBRARIES} glog::glog) # "${deepinspectapptemplate_ROOT}/deepTemplateExe/deepinspectControl.lib")
    include_directories(${AWSSDK_INCLUDE_DIR})
else()
    message("not use aws")
    add_executable(${PROJECT_NAME}
        ${my_Sources}
        )
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${my_Libs}
        )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
#message(${PROJECT_NAME})

#include_directories("${deepinspectapptemplate_ROOT}/deepinspectControl")
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${REA_INCLUDE_DIRS})

#include_directories(${RxCpp_INCLUDE_DIRS})
#include_directories(${RapidJSON_INCLUDE_DIRS})
#include_directories(${PYTHON_INCLUDE_DIRS})
#include_directories(${DeepInspectSDK_INCLUDE_DIRS})
#message("xxx: ${REA_INCLUDE_DIRS}")
#include(${VTK_USE_FILE})
#include_directories("D:/thirdParty/tinydnn/tiny-dnn")
#include_directories(${RapidJSON_INCLUDE_DIRS})
#include_directories(${RxCpp_INCLUDE_DIRS})
#include_directories (${gflags_INCLUDE_DIR})
#include_directories("D:/catch/Catch2/single_include/catch2")

#message("include ${RapidJSON_INCLUDE_DIRS}")
#message("include: ${RapidJSON_INCLUDE_DIRS}")
#message("library: ${REA_LIBS}")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resource"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "copying resource.")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/../frm/install/bin/${CMAKE_BUILD_TYPE}"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "copying framework.")
