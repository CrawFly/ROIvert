// What a MESS! this clearly needs a detailed pass

#include <QDebug>


#include "roivert.h"

#include "qboxlayout.h"
#include "qpushbutton.h"
#include "qtoolbox.h"
#include "qformlayout.h"
#include "qcheckbox.h"
#include "qspinbox.h"
#include "qdockwidget.h"
#include "qprogressdialog.h"
#include "displaysettings.h"
#include "qtextstream.h"
#include "qactiongroup.h"
#include "qmessagebox.h"
#include "qsettings.h"
#include <QGraphicsLayout>
#include "TraceView.h"
#include "opencv2/opencv.hpp"
#include "ROIVertEnums.h"
#include <QStringList>
#include "ROI/ROIStyle.h"
#include "widgets/TraceChartWidget.h"

Roivert::Roivert(QWidget* parent)
    : QMainWindow(parent)
{

    //todo: megarefactor!
    ui.setupUi(this);
    viddata = new VideoData();

    setStyleSheet("QMainWindow::separator { background-color: #bbb; width: 1px; height: 1px; }");

    QGridLayout* gridLayout = new QGridLayout(ui.centralWidget); // top level layout that makes everything stretch-to-fit
    setDockNestingEnabled(true);

    t_imgData = new tool::imgData(this);
    w_imgData = new QDockWidget();
    w_imgData->setWidget(t_imgData);
    w_imgData->setWindowTitle("Image Data");
    w_imgData->setObjectName("WImageData");
    addDockWidget(Qt::RightDockWidgetArea, w_imgData);

    t_imgSettings = new tool::imgSettings(this);
    w_imgSettings = new QDockWidget();
    w_imgSettings->setWidget(t_imgSettings);
    w_imgSettings->setWindowTitle("Image Settings");
    w_imgSettings->setObjectName("WImageSettings");
    addDockWidget(Qt::RightDockWidgetArea, w_imgSettings);
    w_imgSettings->setVisible(false);

    t_io = new tool::fileIO(this);
    w_io = new QDockWidget();
    w_io->setWidget(t_io);
    w_io->setWindowTitle("Import/Export");
    w_io->setObjectName("WImportExport");
    addDockWidget(Qt::RightDockWidgetArea, w_io);
    t_io->setEnabled(false);
    w_io->setVisible(false);

    stylewindow = new StyleWindow(this);
    w_stylewindow = new QDockWidget();
    w_stylewindow->setWindowTitle("Color and Style");
    w_stylewindow->setObjectName("WStyle");
    w_stylewindow->setWidget(stylewindow);

    addDockWidget(Qt::RightDockWidgetArea, w_stylewindow);
    w_stylewindow->setVisible(false);
    //todo: set initial style

    // Right Side:
    QWidget* rightLayoutWidget = new QWidget(ui.centralWidget);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightLayoutWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    // Image Viewer:
    imageview = new ImageView(rightLayoutWidget);
    imageview->setEnabled(false);
    rightLayout->addWidget(imageview);
    
    
    // Contols:
    vidctrl = new VideoControllerWidget(rightLayoutWidget);
    rightLayout->addWidget(vidctrl);
    gridLayout->addWidget(rightLayoutWidget);

    // Trace Viewer
    w_charts = new QDockWidget;
    w_charts->setWindowTitle("Charts");
    traceview = new TraceView(this);

    w_charts->setObjectName("WCharts");
    w_charts->setWidget(traceview);
    addDockWidget(Qt::BottomDockWidgetArea, w_charts);

    
    // ROIs container
    rois = new ROIs(imageview, traceview, viddata);
    stylewindow->setROIs(rois);
    stylewindow->setTraceView(traceview);


    // File IO
    fileio = new FileIO(rois, traceview, viddata);

    doConnect();

    makeToolbar();

    // Action that resets window state:
    QAction* actResetSettings = new QAction(tr("Reset Layout"));
    actResetSettings->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_R));
    connect(actResetSettings, &QAction::triggered, this, &Roivert::resetSettings);
    addAction(actResetSettings);
    
    setWindowIcon(QIcon(":/icons/icons/GreenCrown.png"));
    resize(800, 900);
    
    restoreSettings();    
    stylewindow->loadSettings();

    
    
}
void Roivert::doConnect() {
    // todo: most of this could be eradicated by just passing some pointers around...is that better? worse?

    // imgdata tool load button hit, initiates loading the video
    connect(t_imgData, &tool::imgData::fileLoadRequested, this, &Roivert::loadVideo);

    // when timer or manual change of frame, initiate draw
    connect(vidctrl, &VideoControllerWidget::frameChanged, this, &Roivert::changeFrame);

    // framerate change (simple) fan out to vidctrl and traceview
    connect(t_imgData, &tool::imgData::frameRateChanged, this, &Roivert::frameRateChanged);

    // mostly destined for displaysettings, change in smoothing/contrast etc.
    connect(t_imgSettings, &tool::imgSettings::imgSettingsChanged, this, &Roivert::imgSettingsChanged);
        
    // todo: (consider) give tool::fileio interface a ptr to fileio so it can call directly
    // File IO:
    connect(t_io, &tool::fileIO::exportTraces, this, [=](QString fn, bool dohdr, bool dotime) {fileio->exportTraces(fn, dohdr, dotime); });
    connect(t_io, &tool::fileIO::exportROIs, this, [=](QString fn) {fileio->exportROIs(fn); });
    connect(t_io, &tool::fileIO::importROIs, this, [=](QString fn) {fileio->importROIs(fn); });
    connect(t_io, &tool::fileIO::exportCharts, this, [=](QString fn, int width, int height, int quality, bool ridge) {fileio->exportCharts(fn,width,height,quality,ridge); });
    

    // progress for loading
    connect(viddata, &VideoData::loadProgress, t_imgData, &tool::imgData::setProgBar);

    // clicked the dff button, update contrast widget
    connect(vidctrl, &VideoControllerWidget::dffToggle, this, &Roivert::updateContrastWidget);



}
void Roivert::loadVideo(const QStringList fileList, const double frameRate, const int dsTime, const int dsSpace)
{

    // Confirm load if rois exist:
    if (rois->getNROIs() > 0) {
        // rois exist:
        QMessageBox msg;
        msg.setText(tr("Existing ROIs will be removed when loading a new file, continue?"));
        msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        const int ret = msg.exec();
        if (ret == QMessageBox::Cancel) {
            return;
        }
        rois->deleteAllROIs();
    }

    viddata->load(fileList, dsTime, dsSpace);
    t_imgData->setProgBar(-1);
    vidctrl->setNFrames(viddata->getNFrames());
    vidctrl->setFrameRate(frameRate / dsTime);
    viddata->setFrameRate(frameRate / dsTime); // duplicated for convenience
    t_imgData->fileLoadCompleted(viddata->getNFrames(), viddata->getHeight(), viddata->getWidth());
    
    vidctrl->setEnabled(true);
    t_imgSettings->setEnabled(true);
    imageview->setEnabled(true);
    t_io->setEnabled(true);

    updateContrastWidget(vidctrl->isDff());
}
void Roivert::changeFrame(const size_t frame)
{
    if (frame > 0 && frame <= viddata->getNFrames())
    { 
        cv::Mat thisframe;
        thisframe = viddata->get(vidctrl->isDff(),dispSettings.getProjectionMode(),frame-1);
        cv::Mat proc = dispSettings.getImage(thisframe, vidctrl->isDff());
        // todo: consider moving fmt to dispSettings
        const QImage::Format fmt = dispSettings.useCmap() ? QImage::Format_BGR888 : QImage::Format_Grayscale8;
        QImage qimg(proc.data,proc.cols,proc.rows,proc.step,fmt);
        imageview->setImage(qimg);
    }
 }
void Roivert::frameRateChanged(double frameRate){
    vidctrl->setFrameRate(frameRate / viddata->getdsTime());
    viddata->setFrameRate(frameRate / viddata->getdsTime()); // duplicated for convenience
    rois->updateROITraces();
}
void Roivert::makeToolbar() {

    QActionGroup* ROIGroup = new QActionGroup(this);

    QAction* actROIEllipse = new QAction(QIcon(":/icons/icons/ROIEllipse.png"), "", ROIGroup);
    QAction* actROIPoly = new QAction(QIcon(":/icons/icons/ROIPoly.png"), "", ROIGroup);
    QAction* actROIRect = new QAction(QIcon(":/icons/icons/ROIRect.png"), "", ROIGroup);
    QAction* actROISelect = new QAction(QIcon(":/icons/icons/ROISelect.png"), "", ROIGroup);

    actROIEllipse->setCheckable(true);
    actROIPoly->setCheckable(true);
    actROIRect->setCheckable(true);
    actROISelect->setCheckable(true);

    actROIEllipse->setChecked(true);
    rois->setROIShape(ROIVert::SHAPE::ELLIPSE);

    actROIEllipse->setProperty("Shape", static_cast<int>(ROIVert::SHAPE::ELLIPSE));
    actROIPoly->setProperty("Shape", static_cast<int>(ROIVert::SHAPE::POLYGON));
    actROIRect->setProperty("Shape", static_cast<int>(ROIVert::SHAPE::RECTANGLE));
    actROISelect->setProperty("Shape", static_cast<int>(ROIVert::SHAPE::SELECT));

    actROIEllipse->setToolTip(tr("Draw ellipse ROIs"));
    actROIPoly->setToolTip(tr("Draw polygon ROIs"));
    actROIRect->setToolTip(tr("Draw rectangle ROIs"));
    actROISelect->setToolTip(tr("Select ROIs"));


    ui.mainToolBar->addActions(ROIGroup->actions());
    ui.mainToolBar->addSeparator();
    connect(ROIGroup, &QActionGroup::triggered, this, [&](QAction* act)
    {
        ROIVert::SHAPE shp{act->property("Shape").toInt()  };
        rois->setROIShape(shp);
    });

    // add dockables...
    w_imgData->toggleViewAction()->setIcon(QIcon(":/icons/icons/t_ImgData.png"));
    ui.mainToolBar->addAction(w_imgData->toggleViewAction());
    w_imgSettings->toggleViewAction()->setIcon(QIcon(":/icons/icons/t_ImgSettings.png"));
    ui.mainToolBar->addAction(w_imgSettings->toggleViewAction());
    w_charts->toggleViewAction()->setIcon(QIcon(":/icons/icons/t_Charts.png"));
    ui.mainToolBar->addAction(w_charts->toggleViewAction());
    w_io->toggleViewAction()->setIcon(QIcon(":/icons/icons/t_io.png"));
    ui.mainToolBar->addAction(w_io->toggleViewAction());
    
    w_stylewindow->toggleViewAction()->setIcon(QIcon(":/icons/icons/t_Colors.png"));
    ui.mainToolBar->addAction(w_stylewindow->toggleViewAction());

    ui.mainToolBar->setFloatable(false);
    ui.mainToolBar->toggleViewAction()->setVisible(false);
    addToolBar(Qt::LeftToolBarArea, ui.mainToolBar);
}
void Roivert::updateContrastWidget(bool isDff) {
    // this sets histogram and contrast on the widget:
    const ROIVert::contrast c = dispSettings.getContrast(isDff);
    t_imgSettings->setContrast(c);

    // todo: consider taking same approach as I did with dispSettings, storing raw and dff in [0] and [1] and using bool to address...
    std::vector<float> hist; 
    viddata->getHistogram(isDff, hist);
    t_imgSettings->setHistogram(hist);
}
void Roivert::imgSettingsChanged(ROIVert::imgsettings settings) {
    
    dispSettings.setContrast(vidctrl->isDff(), settings.Contrast);
    
    dispSettings.setProjectionMode(settings.projectionType);
    if (settings.projectionType > 0) { vidctrl->stop(); }
    vidctrl->setEnabled(settings.projectionType == 0);

    dispSettings.setColormap(settings.cmap);
    dispSettings.setSmoothing(settings.Smoothing);
    
    vidctrl->forceUpdate();
}
void Roivert::selecttoolfromkey(int key) {
    //todo: enable this
    const int item = key - Qt::Key_1;
    if (item >= 0 && item < ui.mainToolBar->actions().size()) {
        QAction* act = ui.mainToolBar->actions()[item];
        act->activate(QAction::Trigger);
    }
}

void Roivert::closeEvent(QCloseEvent* event) {
    QSettings settings("Neuroph", "ROIVert");
    settings.setValue("version", 1.f);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());

    // todo: store style info and chart colors
    const auto rs{ rois->getCoreROIStyle() };
    settings.beginGroup("Style");
        settings.beginGroup("ROIStyle");
            settings.setValue("linewidth", rs->getPen().style()==Qt::NoPen ? 0 : rs->getPen().width());
            settings.setValue("selsize", rs->getSelectorSize());
            settings.setValue("fillopacity", rs->getBrush().style()==Qt::NoBrush ? 0 : rs->getBrush().color().alpha());
        settings.endGroup();
    
        
        auto cls{ traceview->getCoreLineChartStyle() };
        auto crs{ traceview->getCoreRidgeChartStyle() };

        settings.beginGroup("ChartStyle");
            settings.setValue("back", cls->getBackgroundColor().name());
            settings.setValue("fore", cls->getAxisPen().color().name());
            settings.setValue("lblfontsize", cls->getLabelFont().pointSize());
            settings.setValue("tickfontsize", cls->getTickLabelFont().pointSize());
            settings.setValue("font", cls->getTickLabelFont().family());
        settings.endGroup();


        settings.beginGroup("LineChartStyle");
            settings.setValue("width", cls->getTracePen().style() == Qt::NoPen ? 0 : cls->getTracePen().width());
            settings.setValue("fillopacity", cls->getTraceBrush().style() == Qt::NoBrush ? 0 : cls->getTraceBrush().color().alpha());
            settings.setValue("gradient", cls->getTraceFillGradient());
            settings.setValue("grid", cls->getGrid());
            settings.setValue("normalization", static_cast<int>(cls->getNormalization()));
            settings.setValue("matchy", rois->getMatchYAxes());
        settings.endGroup();
        
        settings.beginGroup("RidgeChartStyle");
            settings.setValue("width", crs->getTracePen().style() == Qt::NoPen ? 0 : crs->getTracePen().width());
            settings.setValue("fillopacity", crs->getTraceBrush().style() == Qt::NoBrush ? 0 : crs->getTraceBrush().color().alpha());
            settings.setValue("gradient", crs->getTraceFillGradient());
            settings.setValue("grid", crs->getGrid());
            settings.setValue("overlap", traceview->getRidgeChart().offset);
        settings.endGroup();

    settings.endGroup();
    QMainWindow::closeEvent(event);
}
void Roivert::restoreSettings()
{
    QSettings settings("Neuroph", "ROIVert");

    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    auto cls{ traceview->getCoreLineChartStyle() };
    auto crs{ traceview->getCoreRidgeChartStyle() };

    settings.beginGroup("Style");
        settings.beginGroup("ROIStyle");
            auto rs = rois->getCoreROIStyle();
            if (settings.contains("linewidth")) { rs->setLineWidth(settings.value("linewidth").toInt()); };
            if (settings.contains("selsize")) { rs->setSelectorSize(settings.value("selsize").toInt()); };
            if (settings.contains("fillopacity")) { rs->setFillOpacity(settings.value("fillopacity").toInt()); };
        settings.endGroup();
        
        settings.beginGroup("ChartStyle");
            if (settings.contains("back")) { cls->setBackgroundColor(QColor(settings.value("back").toString())); ;
                                             crs->setBackgroundColor(QColor(settings.value("back").toString())); };
            if (settings.contains("fore")) { cls->setAxisColor(QColor(settings.value("fore").toString())); 
                                             crs->setAxisColor(QColor(settings.value("fore").toString())); };
            if (settings.contains("lblfontsize")) { cls->setLabelFontSize(settings.value("lblfontsize").toInt());
                                                    crs->setLabelFontSize(settings.value("lblfontsize").toInt()); };
            if (settings.contains("tickfontsize")) { cls->setTickLabelFontSize(settings.value("tickfontsize").toInt());
                                                     crs->setTickLabelFontSize(settings.value("tickfontsize").toInt()); };
            if (settings.contains("font")) { cls->setFontFamily(settings.value("font").toString());
                                             crs->setFontFamily(settings.value("font").toString()); };
        settings.endGroup();
        

        settings.beginGroup("LineChartStyle");
            if (settings.contains("width")) { cls->setTraceLineWidth(settings.value("width").toInt()); };
            if (settings.contains("fillopacity")) { cls->setTraceFillOpacity(settings.value("fillopacity").toInt()); };
            if (settings.contains("gradient")) { cls->setTraceFillGradient(settings.value("gradient").toBool()); };
            if (settings.contains("grid")) { cls->setGrid(settings.value("grid").toBool()); };
            if (settings.contains("normalization")) { cls->setNormalization(static_cast<ROIVert::NORMALIZATION>(settings.value("normalization").toInt())); };
            if (settings.contains("matchy")) { rois->setMatchYAxes(settings.value("matchy").toBool()); };

        settings.endGroup();
        
        settings.beginGroup("RidgeChartStyle");
            if (settings.contains("width")) { crs->setTraceLineWidth(settings.value("width").toInt()); };
            if (settings.contains("fillopacity")) { crs->setTraceFillOpacity(settings.value("fillopacity").toInt()); };
            if (settings.contains("gradient")) { crs->setTraceFillGradient(settings.value("gradient").toBool()); };
            if (settings.contains("grid")) { crs->setGrid(settings.value("grid").toBool()); };
            if (settings.contains("overlap")) { traceview->getRidgeChart().offset = settings.value("overlap").toFloat(); };
        settings.endGroup();
    settings.endGroup();
}

void Roivert::resetSettings() {
    // reset core styles:
    auto rs{ rois->getCoreROIStyle() };
    auto cls{ traceview->getCoreLineChartStyle() };
    auto crs{ traceview->getCoreRidgeChartStyle() };
    *rs = ROIStyle();
    *cls = ChartStyle();
    *crs = ChartStyle();
    
    crs->setDoBackBrush(true);
    crs->setNormalization(ROIVert::NORMALIZATION::ZEROTOONE);
    crs->setLimitStyle(ROIVert::LIMITSTYLE::TIGHT);

    rois->setMatchYAxes(false);
    traceview->getRidgeChart().offset = .5;

    // use stylewindow to apply:
    stylewindow->loadSettings();
    stylewindow->ROIStyleChange();
    stylewindow->ChartStyleChange();
    stylewindow->LineChartStyleChange();
    stylewindow->RidgeChartStyleChange();
    stylewindow->RidgeOverlapChange();
    stylewindow->LineMatchyChange();

    // Reset layout:
    // Dock all dockables in default position
    addDockWidget(Qt::BottomDockWidgetArea, w_charts);
    addDockWidget(Qt::RightDockWidgetArea, w_imgData);
    addDockWidget(Qt::RightDockWidgetArea, w_imgSettings);
    addDockWidget(Qt::RightDockWidgetArea, w_io);
    addDockWidget(Qt::RightDockWidgetArea, w_stylewindow);

    w_charts->setFloating(false);
    w_imgData->setFloating(false);
    w_imgSettings->setFloating(false);
    w_io->setFloating(false);
    w_stylewindow->setFloating(false);

    // Set dockables to visible off (except file loader)
    w_charts->setVisible(true);
    w_imgData->setVisible(true);

    w_imgSettings->setVisible(false);
    w_io->setVisible(false);
    w_stylewindow->setVisible(false);

    // Put toolbar at left
    addToolBar(Qt::LeftToolBarArea, ui.mainToolBar);

    qApp->processEvents(QEventLoop::AllEvents);

    // Set window size
    resize(800, 900);
}