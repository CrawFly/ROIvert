// What a MESS! this clearly needs a detailed pass

#include "roivert.h"
#include "ui_roivert.h"

#include <QDebug>
#include <QBoxLayout>
#include <QSettings>
#include <QMessageBox>
#include <QDesktopWidget>

#include "ImageDataWindow.h"
#include "ImageLoadingProgressWindow.h"

#include "DisplaySettings.h"
#include "FileIO.h"
#include "ImageView.h"

#include "ROI/ROIs.h"
#include "ROI/ROIStyle.h"
#include "ROIVertEnums.h"
#include "ROIVertSettings.h"

#include "dockwidgets/ImageDataWidget.h"
#include "dockwidgets/ImageSettingsWidget.h"
#include "dockwidgets/StyleWidget.h"
#include "dockwidgets/FileIOWidget.h"
#include "dockwidgets/TraceViewWidget.h"

#include "VideoData.h"
#include "widgets/VideoControllerWidget.h"
#include "widgets/TraceChartWidget.h"

struct Roivert::pimpl
{
    Ui::RoivertClass ui;

    std::unique_ptr<QVBoxLayout> toplayout{ nullptr };

    std::unique_ptr<VideoData> viddata{ nullptr };
    
    std::unique_ptr<ImageDataWindow> imagedatawindow{ nullptr };

    std::unique_ptr<ImageDataWidget> imagedatawidget{ nullptr };
    std::unique_ptr<ImageSettingsWidget> imagesettingswidget{ nullptr };
    std::unique_ptr<StyleWidget> stylewidget{ nullptr };
    std::unique_ptr<FileIOWidget> fileiowidget{ nullptr };
    std::unique_ptr<TraceViewWidget> traceviewwidget{ nullptr };
    std::unique_ptr<ImageLoadingProgressWindow> imageloadingprogresswindow{ nullptr };

    std::unique_ptr<ImageView> imageview{ nullptr };
    std::unique_ptr<VideoControllerWidget> vidctrl{ nullptr };

    std::unique_ptr<ROIs> rois{ nullptr };
    std::unique_ptr<FileIO> fileio{ nullptr };

    std::unique_ptr<QActionGroup> ROIGroup{ nullptr };
    std::unique_ptr<QAction> actROIEllipse{ nullptr };
    std::unique_ptr<QAction> actROIPoly{ nullptr };
    std::unique_ptr<QAction> actROIRect{ nullptr };
    std::unique_ptr<QAction> actROISelect{ nullptr };
    std::unique_ptr<QAction> actReset{ nullptr };

    std::unique_ptr<ROIVertSettings> roivertsettings{ nullptr };

    DisplaySettings dispSettings;

    void layout();
    void initDockWidgets(Roivert* par);
    void setWidgetParams();
    void makeObjects(Roivert* par);
    void makeToolbar(Roivert* par);

    QSize screensize{ 3840, 2100 };
};

Roivert::Roivert(QWidget* parent) :
    QMainWindow(parent), impl(std::make_unique<pimpl>())
{

    QApplication::setOrganizationName("Neuroph");
    impl->makeObjects(this);
    impl->initDockWidgets(this);
    impl->setWidgetParams();
    impl->layout();

    setStyleSheet("QMainWindow::separator { background-color: #bbb; width: 1px; height: 1px; }");
    setDockNestingEnabled(true);

    doConnect(); // todo: consider move to impl
    impl->makeToolbar(this);
    setWindowIcon(QIcon(":/icons/GreenCrown.png"));

    impl->screensize = QDesktopWidget().availableGeometry(this).size();
}

Roivert::~Roivert() = default;

void Roivert::doConnect()
{
    connect(impl->imagedatawindow.get(), &ImageDataWindow::fileLoadRequested, this, &Roivert::loadVideo);

    connect(impl->vidctrl.get(), &VideoControllerWidget::frameChanged, this, &Roivert::changeFrame);
    connect(impl->imagedatawidget.get(), &ImageDataWidget::frameRateChanged, this, &Roivert::frameRateChanged);
    connect(impl->imagesettingswidget.get(), &ImageSettingsWidget::imgSettingsChanged, this, &Roivert::imgSettingsChanged);

    connect(impl->fileiowidget.get(), &FileIOWidget::exportTraces, [=](QString fn, bool dohdr, bool dotime)
    { impl->fileio->exportTraces(fn, dohdr, dotime); });
    connect(impl->fileiowidget.get(), &FileIOWidget::exportROIs, [=](QString fn)
    { impl->fileio->exportROIs(fn); });
    connect(impl->fileiowidget.get(), &FileIOWidget::importROIs, [=](QString fn)
    { impl->fileio->importROIs(fn); });
    connect(impl->fileiowidget.get(), &FileIOWidget::exportCharts, [=](QString fn, int width, int height, int quality, bool ridge)
    { impl->fileio->exportCharts(fn, width, height, quality, ridge); });

    // progress for loading
    connect(impl->viddata.get(), &VideoData::loadProgress, impl->imageloadingprogresswindow.get(), &ImageLoadingProgressWindow::setProgress);

    // clicked the dff button, update contrast widget
    connect(impl->vidctrl.get(), &VideoControllerWidget::dffToggled, this, &Roivert::updateContrastWidget);
    connect(impl->vidctrl.get(), &VideoControllerWidget::dffToggled, impl->imagesettingswidget.get(), &ImageSettingsWidget::dffToggle);
    connect(impl->imagesettingswidget.get(), &ImageSettingsWidget::dffToggled, impl->vidctrl.get(), &VideoControllerWidget::dffToggle);

    connect(impl->actReset.get(), &QAction::triggered, this, &Roivert::setDefaultGeometry);

    connect(impl->stylewidget.get(), &StyleWidget::ChartStyleChanged, impl->traceviewwidget.get(), &TraceViewWidget::updateMinimumHeight);
}

void Roivert::loadVideo(std::vector<std::pair<QString,size_t>> filenameframelist, const double frameRate, const int dsTime, const int dsSpace)
{

    // Confirm load if rois exist:
    if (impl->rois->size() > 0)
    {
        // rois exist:
        QMessageBox msg;
        msg.setText(tr("Existing ROIs will be removed when loading a new file, continue?"));
        msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        const int ret = msg.exec();
        if (ret == QMessageBox::Cancel)
        {
            return;
        }
        impl->rois->deleteAllROIs();
    }

    impl->imagedatawindow->hide();
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    impl->imageloadingprogresswindow->show();
    impl->viddata->load(filenameframelist, dsTime, dsSpace);
    impl->imageloadingprogresswindow->hide();

    impl->imagedatawidget->setProgBar(-1);
    impl->vidctrl->setNFrames(impl->viddata->getNFrames());
    impl->vidctrl->setFrameRate(frameRate / dsTime);
    impl->viddata->setFrameRate(frameRate / dsTime); // duplicated for convenience

    impl->vidctrl->setEnabled(!impl->imagesettingswidget->isProjectionActive());
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
        thisframe = impl->viddata->get(impl->vidctrl->isDff(), impl->dispSettings.getProjectionMode(), frame - 1);
        cv::Mat proc = impl->dispSettings.getImage(thisframe, impl->vidctrl->isDff());
        // todo: consider moving fmt to dispSettings
        const QImage::Format fmt = impl->dispSettings.useCmap() ? QImage::Format_BGR888 : QImage::Format_Grayscale8;
        QImage qimg(proc.data, proc.cols, proc.rows, proc.step, fmt);
        impl->imageview->setImage(qimg);
    }
}
void Roivert::frameRateChanged(double frameRate)
{
    impl->vidctrl->setFrameRate(frameRate / impl->viddata->getdsTime());
    impl->viddata->setFrameRate(frameRate / impl->viddata->getdsTime());
    impl->traceviewwidget->updateTMax();
    

    impl->rois->updateROITraces();
}

void Roivert::updateContrastWidget(bool isDff)
{
    // this sets histogram and contrast on the widget:
    const ROIVert::contrast c = impl->dispSettings.getContrast(isDff);

    impl->imagesettingswidget->setContrast(c);

    std::vector<float> hist;
    impl->viddata->getHistogram(isDff, hist);
    impl->imagesettingswidget->setHistogram(hist);
}
void Roivert::imgSettingsChanged(ROIVert::imgsettings settings)
{
    impl->dispSettings.setContrast(impl->vidctrl->isDff(), settings.Contrast);

    impl->dispSettings.setProjectionMode(settings.projectionType);
    if (settings.projectionType > 0)
    {
        impl->vidctrl->stop();
    }
    impl->vidctrl->setEnabled(settings.projectionType == 0);

    impl->dispSettings.setColormap(settings.cmap);
    impl->dispSettings.setSmoothing(settings.Smoothing);

    impl->vidctrl->forceUpdate();
}

void Roivert::closeEvent(QCloseEvent * event) {
    // todo: call ROIVertSettings::saveSettings();
    impl->roivertsettings->saveSettings();
}

void Roivert::setInitialSettings(bool restore) {
    impl->roivertsettings->resetSettings();
    if (restore) {
        impl->roivertsettings->restoreSettings();
    }
    impl->vidctrl->setEnabled(false);

    
    impl->imagedatawindow->setGeometry(geometry());
    impl->imagedatawindow->show();
}
void Roivert::setDefaultGeometry() {
    impl->traceviewwidget->setFloating(true);
    impl->imagedatawidget->setFloating(false);
    impl->imagesettingswidget->setFloating(false);
    impl->fileiowidget->setFloating(false);
    impl->stylewidget->setFloating(false);

    impl->traceviewwidget->setVisible(true);
    impl->imagedatawidget->setVisible(true);
    impl->imagesettingswidget->setVisible(false);
    impl->fileiowidget->setVisible(false);
    impl->stylewidget->setVisible(false);

    addDockWidget(Qt::BottomDockWidgetArea, impl->traceviewwidget.get());
    addDockWidget(Qt::RightDockWidgetArea, impl->imagedatawidget.get());
    addDockWidget(Qt::RightDockWidgetArea, impl->imagesettingswidget.get());
    addDockWidget(Qt::RightDockWidgetArea, impl->fileiowidget.get());
    addDockWidget(Qt::RightDockWidgetArea, impl->stylewidget.get());

    addToolBar(Qt::LeftToolBarArea, impl->ui.mainToolBar);
    qApp->processEvents(QEventLoop::AllEvents);

    auto size = std::min(impl->screensize.width() / 3, 1000);
    resize(size * 1.2, size);
    impl->traceviewwidget->resize(size * .8, size * .3);

}

QSize Roivert::getScreenSize() const {
    return impl->screensize;
}

void Roivert::pimpl::makeObjects(Roivert * par)
{
    ui.setupUi(par);
    toplayout = std::make_unique<QVBoxLayout>(ui.centralWidget);
    viddata = std::make_unique<VideoData>(par);

    imagedatawindow = std::make_unique<ImageDataWindow>(par);
    imageloadingprogresswindow = std::make_unique<ImageLoadingProgressWindow>(imagedatawindow.get());
    imagedatawidget = std::make_unique<ImageDataWidget>(par);
    imagesettingswidget = std::make_unique<ImageSettingsWidget>(par, &dispSettings);
    stylewidget = std::make_unique<StyleWidget>(par);
    fileiowidget = std::make_unique<FileIOWidget>(par);
    traceviewwidget = std::make_unique<TraceViewWidget>(par);

    vidctrl = std::make_unique<VideoControllerWidget>(par);
    imageview = std::make_unique<ImageView>(par);

    rois = std::make_unique<ROIs>(imageview.get(), traceviewwidget.get(), viddata.get());
    rois->setParent(par);
    fileio = std::make_unique<FileIO>(rois.get(), traceviewwidget.get(), viddata.get());

    ROIGroup = std::make_unique<QActionGroup>(par);
    actROIEllipse = std::make_unique<QAction>(QIcon(":/icons/ROIEllipse.png"), "", ROIGroup.get());
    actROIPoly = std::make_unique<QAction>(QIcon(":/icons/ROIPoly.png"), "", ROIGroup.get());
    actROIRect = std::make_unique<QAction>(QIcon(":/icons/ROIRect.png"), "", ROIGroup.get());
    actROISelect = std::make_unique<QAction>(QIcon(":/icons/ROISelect.png"), "", ROIGroup.get());

    actReset = std::make_unique<QAction>(QIcon(":/icons/dockreset.png"), "");
    actReset->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_R));
    par->addAction(actReset.get());

    roivertsettings = std::make_unique<ROIVertSettings>(par, imagedatawidget.get(), imagesettingswidget.get(), stylewidget.get(), fileiowidget.get());
}

void Roivert::pimpl::setWidgetParams()
{
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
    vidctrl->setEnabled(false);

    assert(rois != nullptr);
    assert(traceviewwidget != nullptr);
    stylewidget->setROIs(rois.get());
    stylewidget->setTraceView(traceviewwidget.get());
}

void Roivert::pimpl::initDockWidgets(Roivert * par)
{
    par->addDockWidget(Qt::RightDockWidgetArea, imagedatawidget.get());
    par->addDockWidget(Qt::RightDockWidgetArea, imagesettingswidget.get());
    par->addDockWidget(Qt::RightDockWidgetArea, fileiowidget.get());
    par->addDockWidget(Qt::RightDockWidgetArea, stylewidget.get());
    par->addDockWidget(Qt::BottomDockWidgetArea, traceviewwidget.get());

    imagedatawidget->setObjectName("imagedatawidget");
    imagesettingswidget->setObjectName("imagesettingswidget");
    fileiowidget->setObjectName("fileiowidget");
    stylewidget->setObjectName("stylewidget");
    traceviewwidget->setObjectName("traceviewwidget");
}

void Roivert::pimpl::layout()
{
    toplayout->addWidget(imageview.get());
    toplayout->addWidget(vidctrl.get());
}

void Roivert::pimpl::makeToolbar(Roivert * par)
{
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

    actROIEllipse->setShortcut(Qt::Key_1);
    actROIPoly->setShortcut(Qt::Key_2);
    actROIRect->setShortcut(Qt::Key_3);
    actROISelect->setShortcut(Qt::Key_4);

    actROIEllipse->setObjectName("actROIEllipse");
    actROIPoly->setObjectName("actROIPoly");
    actROIRect->setObjectName("actROIRect");
    actROISelect->setObjectName("actROISelect");

    ui.mainToolBar->addActions(ROIGroup->actions());
    ui.mainToolBar->addSeparator();

    connect(ROIGroup.get(), &QActionGroup::triggered, par, [&](QAction* act)
    {
        ROIVert::SHAPE shp{ act->property("Shape").toInt() };
        rois->setROIShape(shp);
    });

    imagedatawidget->toggleViewAction()->setIcon(QIcon(":/icons/t_ImgData.png"));
    imagesettingswidget->toggleViewAction()->setIcon(QIcon(":/icons/t_ImgSettings.png"));
    traceviewwidget->toggleViewAction()->setIcon(QIcon(":/icons/t_Charts.png"));
    fileiowidget->toggleViewAction()->setIcon(QIcon(":/icons/t_io.png"));
    stylewidget->toggleViewAction()->setIcon(QIcon(":/icons/t_Colors.png"));

    ui.mainToolBar->addAction(imagedatawidget->toggleViewAction());
    ui.mainToolBar->addAction(imagesettingswidget->toggleViewAction());
    ui.mainToolBar->addAction(traceviewwidget->toggleViewAction());
    ui.mainToolBar->addAction(fileiowidget->toggleViewAction());
    ui.mainToolBar->addAction(stylewidget->toggleViewAction());

    imagedatawidget->toggleViewAction()->setShortcut(Qt::Key_5);
    imagesettingswidget->toggleViewAction()->setShortcut(Qt::Key_6);
    traceviewwidget->toggleViewAction()->setShortcut(Qt::Key_7);
    fileiowidget->toggleViewAction()->setShortcut(Qt::Key_8);
    stylewidget->toggleViewAction()->setShortcut(Qt::Key_9);

    {
        QWidget* empty = new QWidget();
        empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        ui.mainToolBar->addWidget(empty);
    }

    ui.mainToolBar->addAction(actReset.get());

    ui.mainToolBar->setFloatable(false);
    ui.mainToolBar->toggleViewAction()->setVisible(false);
    par->addToolBar(Qt::LeftToolBarArea, ui.mainToolBar);
}