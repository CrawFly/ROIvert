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
#include "DisplaySettings.h"
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
    viddata = new VideoData(); //todo: this can be uniqueptr


    setStyleSheet("QMainWindow::separator { background-color: #bbb; width: 1px; height: 1px; }");

    QGridLayout* gridLayout = new QGridLayout(ui.centralWidget); // top level layout that makes everything stretch-to-fit
    setDockNestingEnabled(true);

    imagedatawidget = new ImageDataWidget(this);
    imagedatawidget->setWindowTitle("Image Data");
    imagedatawidget->setObjectName("WImageData");
    addDockWidget(Qt::RightDockWidgetArea, imagedatawidget);
    imagedatawidget->setContentsEnabled(true);
    imagedatawidget->setVisible(false);

    imagesettingswidget = new ImageSettingsWidget(this);
    imagesettingswidget->setWindowTitle("Image Settings");
    imagesettingswidget->setObjectName("WImageSettings");
    addDockWidget(Qt::RightDockWidgetArea, imagesettingswidget);
    imagesettingswidget->setContentsEnabled(false);
    imagesettingswidget->setVisible(false);

    fileiowidget = new FileIOWidget(this);
    fileiowidget->setWindowTitle("Import/Export");
    fileiowidget->setObjectName("WImportExport");
    addDockWidget(Qt::RightDockWidgetArea, fileiowidget);
    fileiowidget->setContentsEnabled(false);
    fileiowidget->setVisible(false);

    stylewidget = new StyleWidget(this);
    stylewidget->setWindowTitle("Color and Style");
    stylewidget->setObjectName("WStyle");
    addDockWidget(Qt::RightDockWidgetArea, stylewidget);
    stylewidget->setVisible(false);
    stylewidget->setContentsEnabled(false);

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
    stylewidget->setROIs(rois);
    stylewidget->setTraceView(traceview);


    // File IO
    fileio = new FileIO(rois, traceview, viddata);

    doConnect();

    makeToolbar();

    // Action that resets window state:
    QAction* actResetSettings = new QAction(tr("Reset Layout"));
    actResetSettings->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_R));
    connect(actResetSettings, &QAction::triggered, this, &Roivert::resetSettings);
    addAction(actResetSettings);
    
    setWindowIcon(QIcon(":/icons/GreenCrown.png"));
    resize(800, 900);
    
    restoreSettings();    
    stylewidget->loadSettings();

    
    
}
void Roivert::doConnect() {
    // todo: most of this could be eradicated by just passing some pointers around...is that better? worse?

    // imgdata tool load button hit, initiates loading the video
    connect(imagedatawidget, &ImageDataWidget::fileLoadRequested, this, &Roivert::loadVideo);

    // when timer or manual change of frame, initiate draw
    connect(vidctrl, &VideoControllerWidget::frameChanged, this, &Roivert::changeFrame);

    // framerate change (simple) fan out to vidctrl and traceview
    connect(imagedatawidget, &ImageDataWidget::frameRateChanged, this, &Roivert::frameRateChanged);

    // mostly destined for displaysettings, change in smoothing/contrast etc.
    connect(imagesettingswidget, &ImageSettingsWidget::imgSettingsChanged, this, &Roivert::imgSettingsChanged);
        
    // todo: (consider) give tool::fileio interface a ptr to fileio so it can call directly
    // File IO:
    connect(fileiowidget, &FileIOWidget::exportTraces, this, [=](QString fn, bool dohdr, bool dotime) {fileio->exportTraces(fn, dohdr, dotime); });
    connect(fileiowidget, &FileIOWidget::exportROIs, this, [=](QString fn) {fileio->exportROIs(fn); });
    connect(fileiowidget, &FileIOWidget::importROIs, this, [=](QString fn) {fileio->importROIs(fn); });
    connect(fileiowidget, &FileIOWidget::exportCharts, this, [=](QString fn, int width, int height, int quality, bool ridge) {fileio->exportCharts(fn,width,height,quality,ridge); });
    

    // progress for loading
    connect(viddata, &VideoData::loadProgress, imagedatawidget, &ImageDataWidget::setProgBar);

    // clicked the dff button, update contrast widget
    connect(vidctrl, &VideoControllerWidget::dffToggle, this, &Roivert::updateContrastWidget);


    connect(imageview, &ImageView::keyPressed, this, [=](int key, Qt::KeyboardModifiers mod) {
        if (mod == Qt::KeyboardModifier::NoModifier){
            if (key >= Qt::Key_1 && key <= Qt::Key_4) {
                selecttool(key - Qt::Key_1);
            }
            else if (key >= Qt::Key_5 && key <= Qt::Key_9) {
                selecttool(key - Qt::Key_1 + 1);
            }

        }
    });
    
}
void Roivert::loadVideo(const QStringList fileList, const double frameRate, const int dsTime, const int dsSpace)
{
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
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

    imagedatawidget->setProgBar(-1);
    vidctrl->setNFrames(viddata->getNFrames());
    vidctrl->setFrameRate(frameRate / dsTime);
    viddata->setFrameRate(frameRate / dsTime); // duplicated for convenience

    vidctrl->setEnabled(true);
    imagesettingswidget->setContentsEnabled(true);
    imageview->setEnabled(true);
    stylewidget->setContentsEnabled(true);
    fileiowidget->setContentsEnabled(true);

    updateContrastWidget(vidctrl->isDff());
    QGuiApplication::restoreOverrideCursor();
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

    QAction* actROIEllipse = new QAction(QIcon(":/icons/ROIEllipse.png"), "", ROIGroup);
    QAction* actROIPoly = new QAction(QIcon(":/icons/ROIPoly.png"), "", ROIGroup);
    QAction* actROIRect = new QAction(QIcon(":/icons/ROIRect.png"), "", ROIGroup);
    QAction* actROISelect = new QAction(QIcon(":/icons/ROISelect.png"), "", ROIGroup);

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
        ROIVert::SHAPE shp{act->property("Shape").toInt() };
        rois->setROIShape(shp);
    });

    // add dockables...
    imagedatawidget->toggleViewAction()->setIcon(QIcon(":/icons/t_ImgData.png"));
    ui.mainToolBar->addAction(imagedatawidget->toggleViewAction());
    imagesettingswidget->toggleViewAction()->setIcon(QIcon(":/icons/t_ImgSettings.png"));
    ui.mainToolBar->addAction(imagesettingswidget->toggleViewAction());
    w_charts->toggleViewAction()->setIcon(QIcon(":/icons/t_Charts.png"));
    ui.mainToolBar->addAction(w_charts->toggleViewAction());
    fileiowidget->toggleViewAction()->setIcon(QIcon(":/icons/t_io.png"));
    ui.mainToolBar->addAction(fileiowidget->toggleViewAction());
    
    stylewidget->toggleViewAction()->setIcon(QIcon(":/icons/t_Colors.png"));
    ui.mainToolBar->addAction(stylewidget->toggleViewAction());

    ui.mainToolBar->setFloatable(false);
    ui.mainToolBar->toggleViewAction()->setVisible(false);
    addToolBar(Qt::LeftToolBarArea, ui.mainToolBar);
}
void Roivert::updateContrastWidget(bool isDff) {
    // this sets histogram and contrast on the widget:
    const ROIVert::contrast c = dispSettings.getContrast(isDff);
    imagesettingswidget->setContrast(c);

    std::vector<float> hist; 
    viddata->getHistogram(isDff, hist);
    imagesettingswidget->setHistogram(hist);
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
void Roivert::selecttool(int item) {
    if (item >= 0 && item < ui.mainToolBar->actions().size()) {
        QAction* act = ui.mainToolBar->actions()[item];
        if (act) { act->activate(QAction::Trigger); }
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
    stylewidget->loadSettings();
    stylewidget->ROIStyleChange();
    stylewidget->ChartStyleChange();
    stylewidget->LineChartStyleChange();
    stylewidget->RidgeChartStyleChange();
    stylewidget->RidgeOverlapChange();
    stylewidget->LineMatchyChange();

    // Reset layout:
    // Dock all dockables in default position
    addDockWidget(Qt::BottomDockWidgetArea, w_charts);
    addDockWidget(Qt::RightDockWidgetArea, imagedatawidget);
    addDockWidget(Qt::RightDockWidgetArea, imagesettingswidget);
    addDockWidget(Qt::RightDockWidgetArea, fileiowidget);
    addDockWidget(Qt::RightDockWidgetArea, stylewidget);

    w_charts->setFloating(false);
    imagedatawidget->setFloating(false);
    imagesettingswidget->setFloating(false);
    fileiowidget->setFloating(false);
    stylewidget->setFloating(false);

    // Set dockables to visible off (except file loader)
    w_charts->setVisible(true);
    imagedatawidget->setVisible(true);

    imagesettingswidget->setVisible(false);
    fileiowidget->setVisible(false);
    stylewidget->setVisible(false);

    // Put toolbar at left
    addToolBar(Qt::LeftToolBarArea, ui.mainToolBar);

    qApp->processEvents(QEventLoop::AllEvents);

    // Set window size
    resize(800, 900);
}

