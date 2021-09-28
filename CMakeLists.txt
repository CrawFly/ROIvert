cmake_minimum_required(VERSION 3.13)

project(ROIVert VERSION 1.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Test REQUIRED)
find_package(OpenCV 4.4 REQUIRED core imgproc imgcodecs)

message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "")
message(STATUS "Qt library status:")
message(STATUS "    Core Version: ${Qt5Core_VERSION_STRING}")
message(STATUS "    Widgets Version: ${Qt5Widgets_VERSION_STRING}")
message(STATUS "    Gui Version: ${Qt5Gui_VERSION_STRING}")
message(STATUS "    Test Version: ${Qt5Test_VERSION_STRING}")



add_library(ROIVertLib "")
add_executable(ROIVert WIN32 "")
add_executable(ROIVertTest "")

# seems like the qrc should be in lib, but it works this way for some reason?
target_sources(ROIVert 
    PRIVATE
        resource.h
        roivert.qrc
        Roivert.rc
)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test)

target_link_libraries(ROIVertLib PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui )
target_link_libraries(ROIVert PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui )
target_link_libraries(ROIVertTest PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Test )

target_link_libraries(ROIVertLib PRIVATE ${OpenCV_LIBS})
target_link_libraries(ROIVert PRIVATE ROIVertLib)
target_link_libraries(ROIVertTest PRIVATE ROIVertLib)
