// What a MESS! this clearly needs a detailed pass

#include "roivert.h"

#include <QDebug>

#include "ui_roivert.h"
#include "videodata.h"
#include "widgets/VideoControllerWidget.h"
#include "dockwidgets/ImageDataWidget.h"
#include "dockwidgets/ImageSettingsWidget.h"
#include "dockwidgets/StyleWidget.h"
#include "dockwidgets/FileIOWidget.h"
#include "dockwidgets/TraceViewWidget.h"
#include "ROI/ROIs.h"
#include "FileIO.h"


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
#include "opencv2/opencv.hpp"
#include "ROIVertEnums.h"
#include <QStringList>
#include "ROI/ROIStyle.h"
#include "widgets/TraceChartWidget.h"

/*
* A note about memory in ROIVert:
*   Leaks aren't a big concern for many of the widgets and other members in ROIVert, as their lifetime starts and ends with ROIVert.
*   Nonetheless, it seems prudent to try to recover allocated memory in a normal fashion. 
*   HOWEVER, the picture is muddied by Qt's ownership rules:
*       A QObject's that has children will automatically delete each child during destruction.
* 
*   One might just create all parented QObjects (all QWidgets) with new, but this is unsettling: it means paying close attention to whether 
*   each member is managed by Qt or not.
*   One might make everything a unique_ptr (or keep things on the stack) relying on scope to address deletion. But this introduces a bug:
*       - if an object's parent is created first, and the object is created second, everything is fine: the child is destructed, Qt removes 
*       it from the parent, and when the parent is destructed everything is fine.
*       - but if an object is created first, and then the parent, a problem is raised. The parent is destructed first, causing the child to 
*       destructed, but the unique_ptr remains. When the variable goes out of scope, the destructor is called a second time. 
*   
*  I don't see threeish solutions: 
*       - pay attention to ownership, creating objects which Qt will manage with new
*       - pay attention to order, always creating parents before children, use unique_ptr's broadly (or stack)
*       - create as a unique_ptr, but release upon parenting...but this means that the unique ptr will be nullptr. 
* 
*  ROIVert was initially written with the first approach, which had no real functional issues, but may have created undetected memory leaks.
*  ROIVert 1.0 moves toward the second option, with the hopes of locking down leaks.
*/

struct Roivert::pimpl {
    Ui::RoivertClass ui; // todo: this is doing next to nothing, can we just drop it now?


    std::unique_ptr<QVBoxLayout> toplayout{ nullptr };

    std::unique_ptr<VideoData> viddata{ nullptr };
    std::unique_ptr<ImageDataWidget> imagedatawidget{ nullptr };
    std::unique_ptr<ImageSettingsWidget> imagesettingswidget{ nullptr };
    std::unique_ptr<StyleWidget> stylewidget{ nullptr };
    std::unique_ptr<FileIOWidget> fileiowidget{ nullptr };
    std::unique_ptr<TraceViewWidget> traceviewwidget{ nullptr };

    std::unique_ptr<ImageView> imageview{ nullptr };
    std::unique_ptr<VideoControllerWidget> vidctrl{ nullptr };

    std::unique_ptr<ROIs> rois{ nullptr };
    std::unique_ptr<FileIO> fileio{ nullptr };
    
    DisplaySettings dispSettings;

    void makeObjects(Roivert* par) {
        ui.setupUi(par);
        toplayout = std::make_unique<QVBoxLayout>(ui.centralWidget);
        viddata = std::make_unique<VideoData>(par);
        
        imagedatawidget = std::make_unique<ImageDataWidget>(par);
        imagesettingswidget = std::make_unique<ImageSettingsWidget>(par);
        stylewidget = std::make_unique<StyleWidget>(par);
        fileiowidget = std::make_unique<FileIOWidget>(par);
        traceviewwidget = std::make_unique<TraceViewWidget>(par);

        vidctrl = std::make_unique<VideoControllerWidget>(par);
        imageview = std::make_unique<ImageView>(par);

        rois = std::make_unique<ROIs>(imageview.get(), traceviewwidget.get(), viddata.get());
        fileio = std::make_unique<FileIO>(rois.get(), traceviewwidget.get(), viddata.get());

    }

    void setWidgetParams() {
        imagedatawidget->setWindowTitle("Image Data");
        imagedatawidget->setContentsEnabled(true);
        imagedatawidget->setVisible(false);

        imagesettingswidget->setWindowTitle("Image Settings");
        imagesettingswidget->setContentsEnabled(false);
        imagesettingswidget->setVisible(false);
        
        fileiowidget->setWindowTitle("Import/Export");
        fileiowidget->setContentsEnabled(false);
        fileiowidget->setVisible(false);
    
        stylewidget->setWindowTitle("Color and Style");
        stylewidget->setVisible(false);
        stylewidget->setContentsEnabled(false);

        traceviewwidget->setWindowTitle("Charts");
        traceviewwidget->setVisible(false);

        toplayout->setContentsMargins(0, 0, 0, 0);

        imageview->setEnabled(false);


    }

    void initDockWidgets(Roivert* par) {
        par->addDockWidget(Qt::RightDockWidgetArea, imagedatawidget.get());
        par->addDockWidget(Qt::RightDockWidgetArea, imagesettingswidget.get());
        par->addDockWidget(Qt::RightDockWidgetArea, fileiowidget.get());
        par->addDockWidget(Qt::RightDockWidgetArea, stylewidget.get());
        par->addDockWidget(Qt::BottomDockWidgetArea, traceviewwidget.get());
    }

    void layout() {
        toplayout->addWidget(imageview.get());
        toplayout->addWidget(vidctrl.get());
    }
};


Roivert::Roivert(QWidget* parent)
    : QMainWindow(parent)
{
    impl->makeObjects(this);
    impl->initDockWidgets(this);
    impl->setWidgetParams();
    impl->layout();

    setStyleSheet("QMainWindow::separator { background-color: #bbb; width: 1px; height: 1px; }");
    setDockNestingEnabled(true);

    // ROIs container
    impl->stylewidget->setROIs(impl->rois.get());
    impl->stylewidget->setTraceView(impl->traceviewwidget.get());

    // File IO

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
    impl->stylewidget->loadSettings();
}

Roivert::~Roivert() = default;

void Roivert::doConnect() {
    connect(impl->imagedatawidget.get(), &ImageDataWidget::fileLoadRequested, this, &Roivert::loadVideo);
    connect(impl->vidctrl.get(), &VideoControllerWidget::frameChanged, this, &Roivert::changeFrame);
    connect(impl->imagedatawidget.get(), &ImageDataWidget::frameRateChanged, this, &Roivert::frameRateChanged);
    connect(impl->imagesettingswidget.get(), &ImageSettingsWidget::imgSettingsChanged, this, &Roivert::imgSettingsChanged);
        
    // todo: (consider) give fileiowidget interface a ptr to fileio so it can call directly
    connect(impl->fileiowidget.get(), &FileIOWidget::exportTraces, this, [=](QString fn, bool dohdr, bool dotime) {impl->fileio->exportTraces(fn, dohdr, dotime); });
    connect(impl->fileiowidget.get(), &FileIOWidget::exportROIs, this, [=](QString fn) {impl->fileio->exportROIs(fn); });
    connect(impl->fileiowidget.get(), &FileIOWidget::importROIs, this, [=](QString fn) {impl->fileio->importROIs(fn); });
    connect(impl->fileiowidget.get(), &FileIOWidget::exportCharts, this, [=](QString fn, int width, int height, int quality, bool ridge) {impl->fileio->exportCharts(fn,width,height,quality,ridge); });
    

    // progress for loading
    connect(impl->viddata.get(), &VideoData::loadProgress, impl->imagedatawidget.get(), &ImageDataWidget::setProgBar);

    // clicked the dff button, update contrast widget
    connect(impl->vidctrl.get(), &VideoControllerWidget::dffToggle, this, &Roivert::updateContrastWidget);


    connect(impl->imageview.get(), &ImageView::keyPressed, this, [=](int key, Qt::KeyboardModifiers mod) {
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
    // Confirm load if rois exist:
    if (impl->rois->getNROIs() > 0) {
        // rois exist:
        QMessageBox msg;
        msg.setText(tr("Existing ROIs will be removed when loading a new file, continue?"));
        msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        const int ret = msg.exec();
        if (ret == QMessageBox::Cancel) {
            return;
        }
        impl->rois->deleteAllROIs();
    }
    
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    impl->viddata->load(fileList, dsTime, dsSpace);

    impl->imagedatawidget->setProgBar(-1);
    impl->vidctrl->setNFrames(impl->viddata->getNFrames());
    impl->vidctrl->setFrameRate(frameRate / dsTime);
    impl->viddata->setFrameRate(frameRate / dsTime); // duplicated for convenience

    impl->vidctrl->setEnabled(true);
    impl->imagesettingswidget->setContentsEnabled(true);
    impl->imageview->setEnabled(true);
    impl->stylewidget->setContentsEnabled(true);
    impl->fileiowidget->setContentsEnabled(true);

    updateContrastWidget(impl->vidctrl->isDff());
    QGuiApplication::restoreOverrideCursor();
}

void Roivert::changeFrame(const size_t frame)
{
    if (frame > 0 && frame <= impl->viddata->getNFrames())
    { 
        cv::Mat thisframe;
        thisframe = impl->viddata->get(impl->vidctrl->isDff(),impl->dispSettings.getProjectionMode(),frame-1);
        cv::Mat proc = impl->dispSettings.getImage(thisframe, impl->vidctrl->isDff());
        // todo: consider moving fmt to dispSettings
        const QImage::Format fmt = impl->dispSettings.useCmap() ? QImage::Format_BGR888 : QImage::Format_Grayscale8;
        QImage qimg(proc.data,proc.cols,proc.rows,proc.step,fmt);
        impl->imageview->setImage(qimg);
    }
 }
void Roivert::frameRateChanged(double frameRate){
    impl->vidctrl->setFrameRate(frameRate / impl->viddata->getdsTime());
    impl->viddata->setFrameRate(frameRate / impl->viddata->getdsTime()); // duplicated for convenience
    impl->rois->updateROITraces();
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
    impl->rois->setROIShape(ROIVert::SHAPE::ELLIPSE);

    actROIEllipse->setProperty("Shape", static_cast<int>(ROIVert::SHAPE::ELLIPSE));
    actROIPoly->setProperty("Shape", static_cast<int>(ROIVert::SHAPE::POLYGON));
    actROIRect->setProperty("Shape", static_cast<int>(ROIVert::SHAPE::RECTANGLE));
    actROISelect->setProperty("Shape", static_cast<int>(ROIVert::SHAPE::SELECT));

    actROIEllipse->setToolTip(tr("Draw ellipse ROIs"));
    actROIPoly->setToolTip(tr("Draw polygon ROIs"));
    actROIRect->setToolTip(tr("Draw rectangle ROIs"));
    actROISelect->setToolTip(tr("Select ROIs"));


    impl->ui.mainToolBar->addActions(ROIGroup->actions());
    impl->ui.mainToolBar->addSeparator();
    connect(ROIGroup, &QActionGroup::triggered, this, [&](QAction* act)
    {
        ROIVert::SHAPE shp{act->property("Shape").toInt() };
        impl->rois->setROIShape(shp);
    });

    // add dockables...
    impl->imagedatawidget->toggleViewAction()->setIcon(QIcon(":/icons/t_ImgData.png"));
    impl->ui.mainToolBar->addAction(impl->imagedatawidget->toggleViewAction());
    impl->imagesettingswidget->toggleViewAction()->setIcon(QIcon(":/icons/t_ImgSettings.png"));
    impl->ui.mainToolBar->addAction(impl->imagesettingswidget->toggleViewAction());
    impl->traceviewwidget->toggleViewAction()->setIcon(QIcon(":/icons/t_Charts.png"));
    impl->ui.mainToolBar->addAction(impl->traceviewwidget->toggleViewAction());
    impl->fileiowidget->toggleViewAction()->setIcon(QIcon(":/icons/t_io.png"));
    impl->ui.mainToolBar->addAction(impl->fileiowidget->toggleViewAction());
    
    impl->stylewidget->toggleViewAction()->setIcon(QIcon(":/icons/t_Colors.png"));
    impl->ui.mainToolBar->addAction(impl->stylewidget->toggleViewAction());

    impl->ui.mainToolBar->setFloatable(false);
    impl->ui.mainToolBar->toggleViewAction()->setVisible(false);
    addToolBar(Qt::LeftToolBarArea, impl->ui.mainToolBar);
}
void Roivert::updateContrastWidget(bool isDff) {
    // this sets histogram and contrast on the widget:
    const ROIVert::contrast c = impl->dispSettings.getContrast(isDff);
    impl->imagesettingswidget->setContrast(c);

    std::vector<float> hist; 
    impl->viddata->getHistogram(isDff, hist);
    impl->imagesettingswidget->setHistogram(hist);
}
void Roivert::imgSettingsChanged(ROIVert::imgsettings settings) {
    
    impl->dispSettings.setContrast(impl->vidctrl->isDff(), settings.Contrast);
    
    impl->dispSettings.setProjectionMode(settings.projectionType);
    if (settings.projectionType > 0) { impl->vidctrl->stop(); }
    impl->vidctrl->setEnabled(settings.projectionType == 0);

    impl->dispSettings.setColormap(settings.cmap);
    impl->dispSettings.setSmoothing(settings.Smoothing);
    
    impl->vidctrl->forceUpdate();
}
void Roivert::selecttool(int item) {
    if (item >= 0 && item < impl->ui.mainToolBar->actions().size()) {
        QAction* act = impl->ui.mainToolBar->actions()[item];
        if (act) { act->activate(QAction::Trigger); }
    }
}
void Roivert::closeEvent(QCloseEvent* event) {
    QSettings settings("Neuroph", "ROIVert");
    settings.setValue("version", 1.f);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());

    // todo: store style info and chart colors
    const auto rs{ impl->rois->getCoreROIStyle() };
    settings.beginGroup("Style");
        settings.beginGroup("ROIStyle");
            settings.setValue("linewidth", rs->getPen().style()==Qt::NoPen ? 0 : rs->getPen().width());
            settings.setValue("selsize", rs->getSelectorSize());
            settings.setValue("fillopacity", rs->getBrush().style()==Qt::NoBrush ? 0 : rs->getBrush().color().alpha());
        settings.endGroup();
    
        
        auto cls{ impl->traceviewwidget->getCoreLineChartStyle() };
        auto crs{ impl->traceviewwidget->getCoreRidgeChartStyle() };

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
            settings.setValue("matchy", impl->rois->getMatchYAxes());
        settings.endGroup();
        
        settings.beginGroup("RidgeChartStyle");
            settings.setValue("width", crs->getTracePen().style() == Qt::NoPen ? 0 : crs->getTracePen().width());
            settings.setValue("fillopacity", crs->getTraceBrush().style() == Qt::NoBrush ? 0 : crs->getTraceBrush().color().alpha());
            settings.setValue("gradient", crs->getTraceFillGradient());
            settings.setValue("grid", crs->getGrid());
            settings.setValue("overlap", impl->traceviewwidget->getRidgeChart().offset);
        settings.endGroup();

    settings.endGroup();
    QMainWindow::closeEvent(event);
}

void Roivert::restoreSettings()
{
    QSettings settings("Neuroph", "ROIVert");

    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    auto cls{ impl->traceviewwidget->getCoreLineChartStyle() };
    auto crs{ impl->traceviewwidget->getCoreRidgeChartStyle() };

    settings.beginGroup("Style");
        settings.beginGroup("ROIStyle");
            auto rs = impl->rois->getCoreROIStyle();
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
            if (settings.contains("matchy")) { impl->rois->setMatchYAxes(settings.value("matchy").toBool()); };

        settings.endGroup();
        
        settings.beginGroup("RidgeChartStyle");
            if (settings.contains("width")) { crs->setTraceLineWidth(settings.value("width").toInt()); };
            if (settings.contains("fillopacity")) { crs->setTraceFillOpacity(settings.value("fillopacity").toInt()); };
            if (settings.contains("gradient")) { crs->setTraceFillGradient(settings.value("gradient").toBool()); };
            if (settings.contains("grid")) { crs->setGrid(settings.value("grid").toBool()); };
            if (settings.contains("overlap")) { impl->traceviewwidget->getRidgeChart().offset = settings.value("overlap").toFloat(); };
        settings.endGroup();
    settings.endGroup();
}
void Roivert::resetSettings() {
    // reset core styles:
    auto rs{ impl->rois->getCoreROIStyle() };
    auto cls{ impl->traceviewwidget->getCoreLineChartStyle() };
    auto crs{ impl->traceviewwidget->getCoreRidgeChartStyle() };
    *rs = ROIStyle();
    *cls = ChartStyle();
    *crs = ChartStyle();
    
    crs->setDoBackBrush(true);
    crs->setNormalization(ROIVert::NORMALIZATION::ZEROTOONE);
    crs->setLimitStyle(ROIVert::LIMITSTYLE::TIGHT);

    impl->rois->setMatchYAxes(false);
    impl->traceviewwidget->getRidgeChart().offset = .5;

    // use stylewindow to apply:
    impl->stylewidget->loadSettings();
    impl->stylewidget->ROIStyleChange();
    impl->stylewidget->ChartStyleChange();
    impl->stylewidget->LineChartStyleChange();
    impl->stylewidget->RidgeChartStyleChange();
    impl->stylewidget->RidgeOverlapChange();
    impl->stylewidget->LineMatchyChange();

    // Reset layout:
    // Dock all dockables in default position
    addDockWidget(Qt::BottomDockWidgetArea, impl->traceviewwidget.get());
    addDockWidget(Qt::RightDockWidgetArea, impl->imagedatawidget.get());
    addDockWidget(Qt::RightDockWidgetArea, impl->imagesettingswidget.get());
    addDockWidget(Qt::RightDockWidgetArea, impl->fileiowidget.get());
    addDockWidget(Qt::RightDockWidgetArea, impl->stylewidget.get());

    impl->traceviewwidget->setFloating(false);
    impl->imagedatawidget->setFloating(false);
    impl->imagesettingswidget->setFloating(false);
    impl->fileiowidget->setFloating(false);
    impl->stylewidget->setFloating(false);

    // Set dockables to visible off (except file loader)
    impl->imagedatawidget->setVisible(true);

    impl->imagesettingswidget->setVisible(false);
    impl->fileiowidget->setVisible(false);
    impl->stylewidget->setVisible(false);

    // Put toolbar at left
    addToolBar(Qt::LeftToolBarArea, impl->ui.mainToolBar);

    qApp->processEvents(QEventLoop::AllEvents);

    // Set window size
    resize(800, 900);
}

