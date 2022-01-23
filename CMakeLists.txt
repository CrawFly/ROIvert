cmake_minimum_required(VERSION 3.13)

project(ROIVert VERSION 1.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Test REQUIRED)
find_package(OpenCV 4.4 REQUIRED core imgproc imgcodecs)
find_package(TinyTIFFShared REQUIRED)

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
message(STATUS "Tiff Library status:")
message(STATUS "    Found: ${TinyTIFFShared_FOUND}")
message(STATUS "    Location: ${TinyTIFFShared_DIR}")

add_library(ROIVertLib "")
add_executable(ROIVert WIN32 MACOSX_BUNDLE "")
add_executable(ROIVertTest "")

target_sources(ROIVert 
    PRIVATE
        resource.h
        roivert.qrc
)

if(WIN32)
    target_sources(ROIVert
        PRIVATE
            Roivert.rc
    )

    get_target_property(cvcoredll_dbg opencv_core IMPORTED_LOCATION_DEBUG)
    get_target_property(cvimgprocdll_dbg opencv_imgproc IMPORTED_LOCATION_DEBUG)
    get_target_property(cvimgcodecsdll_dbg opencv_imgcodecs IMPORTED_LOCATION_DEBUG)
    get_target_property(cvcoredll_rel opencv_core IMPORTED_LOCATION_RELEASE)
    get_target_property(cvimgprocdll_rel opencv_imgproc IMPORTED_LOCATION_RELEASE)
    get_target_property(cvimgcodecsdll_rel opencv_imgcodecs IMPORTED_LOCATION_RELEASE)

    add_custom_command(TARGET ROIVert POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:ROIVert>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:ROIVert>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:ROIVert>
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ROIVert>/platforms/
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::QWindowsIntegrationPlugin> $<TARGET_FILE_DIR:ROIVert>/platforms/
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ROIVert>/imageformats/
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::QJpegPlugin> $<TARGET_FILE_DIR:ROIVert>/imageformats/
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ROIVert>/styles/
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::QWindowsVistaStylePlugin> $<TARGET_FILE_DIR:ROIVert>/styles/
        
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<$<CONFIG:debug>:${cvcoredll_dbg}>$<$<CONFIG:release>:${cvcoredll_rel}>" $<TARGET_FILE_DIR:ROIVert> 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<$<CONFIG:debug>:${cvimgprocdll_dbg}>$<$<CONFIG:release>:${cvimgprocdll_rel}>" $<TARGET_FILE_DIR:ROIVert> 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<$<CONFIG:debug>:${cvimgcodecsdll_dbg}>$<$<CONFIG:release>:${cvimgcodecsdll_rel}>" $<TARGET_FILE_DIR:ROIVert> 
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${TinyTIFFShared_DIR}\\..\\..\\..\\bin\\$<$<CONFIG:debug>:TinyTIFFShared_Debug.dll>$<$<CONFIG:release>:TinyTIFFShared_Release.dll>" $<TARGET_FILE_DIR:ROIVert>
    )

    add_custom_command(TARGET ROIVertTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Test> $<TARGET_FILE_DIR:ROIVert>
    )
endif()

add_custom_command(TARGET ROIVertTest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test_resources $<TARGET_FILE_DIR:ROIVertTest>/test_resources)
target_compile_definitions(ROIVertTest PUBLIC TEST_RESOURCE_DIR="$<TARGET_FILE_DIR:ROIVertTest>/test_resources")

add_compile_definitions(ROIVERTVERSION="${CMAKE_PROJECT_VERSION}")

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test)

target_link_libraries(ROIVertLib PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui TinyTIFFShared)
target_link_libraries(ROIVert PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui )
target_link_libraries(ROIVertTest PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Test )

target_link_libraries(ROIVertLib PRIVATE ${OpenCV_LIBS})
target_link_libraries(ROIVert PRIVATE ROIVertLib)
target_link_libraries(ROIVertTest PRIVATE ROIVertLib ${OpenCV_LIBS})

##Last ditch snippet for listing all variables
#
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
#