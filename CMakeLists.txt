cmake_minimum_required(VERSION 3.12)
project(LabelRoboMaster)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_VERSION 5)
set(REQUIRED_LIBS Core Gui Widgets Svg)
set(REQUIRED_LIBS_QUALIFIED Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Svg)

find_package(Qt${QT_VERSION} 5.12 COMPONENTS ${REQUIRED_LIBS} REQUIRED)
find_package(OpenCV 4 REQUIRED)

include_directories(.)

aux_source_directory(. src)

qt5_add_big_resources(resource resource.qrc)
add_executable(${PROJECT_NAME} ${src} ${resource})

target_link_libraries(${PROJECT_NAME} ${REQUIRED_LIBS_QUALIFIED})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
