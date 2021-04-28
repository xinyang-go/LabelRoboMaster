cmake_minimum_required(VERSION 3.12)
project(LabelRoboMaster)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_VERSION 5)
set(REQUIRED_LIBS Core Gui Widgets Svg)
set(REQUIRED_LIBS_QUALIFIED Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Svg)

include_directories(.)

aux_source_directory(. src)

add_executable(${PROJECT_NAME} ${src} resource.qrc)

find_package(Qt${QT_VERSION} COMPONENTS ${REQUIRED_LIBS} REQUIRED)
target_link_libraries(${PROJECT_NAME} ${REQUIRED_LIBS_QUALIFIED})

find_package(OpenCV 4 REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
