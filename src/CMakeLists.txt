target_sources(ROIVertLib
    PRIVATE
        ChartStyle.cpp
        DisplaySettings.cpp
        Settingsable.cpp
        DialogWithSettings.cpp
        DockWidgetWithSettings.cpp
        FileIO.cpp
        ImageDataTableModel.cpp
        ImageDataWindow.cpp
        ImageView.cpp
        ROIVertSettings.cpp
        VideoData.cpp
        ZoomPan.cpp
        roivert.cpp
        ImageLoadingProgressWindow.cpp
)

target_sources(ROIVert
    PRIVATE
        main.cpp
)

add_subdirectory(dockwidgets)
add_subdirectory(ROI)
add_subdirectory(widgets)