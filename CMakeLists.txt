cmake_minimum_required(VERSION 3.12)
project(LabelRoboMaster)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_VERSION 5)
set(REQUIRED_LIBS Core Gui Widgets Svg)
set(REQUIRED_LIBS_QUALIFIED Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Svg)

find_package(Qt${QT_VERSION} COMPONENTS ${REQUIRED_LIBS} REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(.)

aux_source_directory(. src)

qt5_add_big_resources(resource resource.qrc)
add_executable(${PROJECT_NAME} ${src} ${resource})

target_link_libraries(${PROJECT_NAME} ${REQUIRED_LIBS_QUALIFIED})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
# 指定包的版本号
set(_VERSION_MAJOR 0)
set(_VERSION_MINOR 8)
set(_VERSION_PATCH 0)
set(CPACK_PACKAGE_VERSION_MAJOR "${_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${_VERSION_PATCH}")
IF(UNIX)
    ##---------------- 以下为用CPack打包 (Ubuntu) 的内容 ----------------##
    # 指定包格式
    set(CPACK_GENERATOR "7Z")
    # 指定包名
    set(CPACK_PACKAGE_NAME "LabelRoboMaster")
    # 指定程序名，就是程序安装后的名字
    set(CPACK_DEBIAN_PACKAGE_NAME "LabelRoboMaster")
    # 指定架构
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    # 设置包作者的联系方式
    set(CPACK_PACKAGE_CONTACT "supporter@gmail.com")
    # 设置维护人
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "members of support@gmail.cn")
    # 指定需要安装的文件
    INSTALL(TARGETS LabelRoboMaster LabelRoboMaster LabelRoboMaster
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )

    # include(CPack)
ELSEIF(WIN32)

ENDIF()
