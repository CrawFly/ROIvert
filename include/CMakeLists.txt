target_sources(ROIVertLib
    PRIVATE
        ChartStyle.h
        DisplaySettings.h
        Settingsable.h
        DialogWithSettings.h
        DockWidgetWithSettings.h
        FileIO.h
        ImageDataWindow.h
        ImageLoadingProgressWindow.h
        ImageView.h
        roivert.h
        roivertcore.h
        ROIVertEnums.h
        ROIVertSettings.h
        VideoData.h
        ZoomPan.h
)

target_include_directories(ROIVertLib PRIVATE ../include)
target_include_directories(ROIVert PUBLIC ../include)
target_include_directories(ROIVertTest PUBLIC ../include)

add_subdirectory(dockwidgets)
add_subdirectory(ROI)
add_subdirectory(widgets)