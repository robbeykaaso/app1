cmake_minimum_required(VERSION 3.1)

project(MYAPP1 LANGUAGES CXX)

add_definitions( -DRX_USE_EXCEPTIONS=OFF )

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
find_package(AWSSDK REQUIRED COMPONENTS core s3)
set(REA_DIR ${CMAKE_CURRENT_LIST_DIR}/../frm/cmake)
#message("xxx: ${CMAKE_CURRENT_LIST_DIR}")
find_package(REA)
#find_package(PythonLibs REQUIRED)

#find_package(DeepInspectSDK REQUIRED)
#find_package(RapidJSON REQUIRED)

#qtquick_compiler_add_resources(RESOURCES qml.qrc)
#find_package(VTK REQUIRED)

add_executable(${PROJECT_NAME}
    "main.cpp" #WIN32: to disable the console window
    "storage/aws_s3.h"
    "storage/aws_s3.cpp"
    "storage/storage.h"
    "storage/storage.cpp"
    "storage/awsStorage.h"
    "storage/awsStorage.cpp"
    "storage/mysql.cpp"
    "socket/protocal.h"
    "socket/server.h"
    "socket/server.cpp"
    "socket/client.h"
    "socket/client.cpp"
    "socket/unitTest.cpp"
    "socket2/logcat.h"
    "socket2/tcpserver.h"
    "socket2/tcpserver.cpp"
    "socket2/tcpsocket.h"
    "socket2/tcpsocket.cpp"
    "serial/modbusMaster.h"
    "serial/modbusMaster.cpp"
    #"device/camera.cpp"
    #"device/io.cpp"
    "util/cv.h"
    "util/cv.cpp"
    "util/pack.cpp"
    "util/image.cpp"
#    "util/py.h"
#    "util/py.cpp"
    "deepsight_anno/model.h"
    "deepsight_anno/model.cpp"
    "deepsight_anno/user.cpp"
    "deepsight_anno/project.cpp"
    "deepsight_anno/task.cpp"
    "deepsight_anno/task.h"
    "deepsight_anno/custom.cpp"
    "qml.qrc"
    #"resource/logo.rc"
    )
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
#message(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core Qt5::Quick Qt5::Widgets Qt5::Sql Qt5::SerialPort Qt5::SerialBus
    ${OpenCV_LIBS}
    ${AWSSDK_LINK_LIBRARIES}
    ${REA_LIBS}
#    ${DeepInspectSDK_LIBS}
#    ${PYTHON_LIBRARIES}
    )# ${VTK_LIBRARIES})# #  ${DeepInspectSDK_LIBS} ${gflags_LIBRARIES} glog::glog) # "${deepinspectapptemplate_ROOT}/deepTemplateExe/deepinspectControl.lib")
#include_directories("${deepinspectapptemplate_ROOT}/deepinspectControl")
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${REA_INCLUDE_DIRS})

include_directories(${RxCpp_INCLUDE_DIRS})
include_directories(${RapidJSON_INCLUDE_DIRS})
#include_directories(${PYTHON_INCLUDE_DIRS})
#include_directories(${DeepInspectSDK_INCLUDE_DIRS})
#message("xxx: ${REA_INCLUDE_DIRS}")
#include(${VTK_USE_FILE})
#include_directories("D:/thirdParty/tinydnn/tiny-dnn")
#include_directories(${RapidJSON_INCLUDE_DIRS})
#include_directories(${RxCpp_INCLUDE_DIRS})
#include_directories (${gflags_INCLUDE_DIR})
#include_directories(${AWSSDK_INCLUDE_DIR})
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
    "${CMAKE_SOURCE_DIR}/../frm/install/bin"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMENT "copying framework.")
