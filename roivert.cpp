#include "roivert.h"
#include "qboxlayout.h"
#include "qsplitter.h"
#include "qpushbutton.h"
#include "qtoolbox.h"
#include "qformlayout.h"
#include "qcheckbox.h"
#include "qspinbox.h"
#include "qdockwidget.h"
#include "qprogressdialog.h"
#include "displaysettings.h"

#include "qactiongroup.h"


Roivert::Roivert(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    viddata = new VideoData(this);

    setStyleSheet("QMainWindow::separator { background-color: #bbb; width: 1px; height: 1px; }");
    
    QGridLayout *gridLayout = new QGridLayout(ui.centralWidget); // top level layout that makes everything stretch-to-fit
    setDockNestingEnabled(true);

    t_imgData = new tool::imgData(this);
    w_imgData = new QDockWidget();
    w_imgData->setWidget(t_imgData);
    w_imgData->setWindowTitle("Image Data");
    addDockWidget(Qt::RightDockWidgetArea, w_imgData);

    t_imgSettings = new tool::imgSettings(this);
    w_imgSettings = new QDockWidget();
    w_imgSettings->setWidget(t_imgSettings);
    w_imgSettings->setWindowTitle("Image Settings");
    addDockWidget(Qt::RightDockWidgetArea, w_imgSettings);

    
    // Right Side:
    QWidget *rightLayoutWidget = new QWidget(ui.centralWidget);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightLayoutWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    // Image Viewer:
    imview = new ImageROIViewer(rightLayoutWidget);
    rightLayout->addWidget(imview);

    Graphics_view_zoom* z = new Graphics_view_zoom(imview);
    z->set_modifiers(Qt::ControlModifier);

    // Contols:
    vidctrl = new VideoController(rightLayoutWidget);
    rightLayout->addWidget(vidctrl);
    gridLayout->addWidget(rightLayoutWidget);

    connect(t_imgData, &tool::imgData::fileLoadRequested, this, &Roivert::loadVideo);
    connect(vidctrl, &VideoController::frameChanged, this, &Roivert::changeFrame);
    connect(viddata, &VideoData::loadProgress, t_imgData, &tool::imgData::setProgBar);
    connect(t_imgData, &tool::imgData::frameRateChanged, this, &Roivert::frameRateChanged);
    
    connect(vidctrl, &VideoController::dffToggle, this, &Roivert::updateContrastWidget);
    
    connect(t_imgSettings, &tool::imgSettings::contrastChanged, this, &Roivert::contrastChange);

    QImage testimage("C:\\Users\\dbulk\\OneDrive\\Documents\\qtprojects\\Roivert\\greenking.png");

    imview->setImage(testimage);
    imview->setMouseMode(ROIVert::ADDROI);
    imview->setROIShape(ROIVert::RECTANGLE);

    makeToolbar();



    setWindowIcon(QIcon(":/icons/icons/GreenCrown.png"));
    resize(800, 550);
}

void Roivert::loadVideo(const QStringList fileList, const double frameRate, const int dsTime, const int dsSpace)
{
    QTime t;
    t.start();
    viddata->load(fileList, dsTime, dsSpace);
    t_imgData->setProgBar(-1);
    qDebug() << "load time: " << t.elapsed()/1000. << "seconds";
    vidctrl->setNFrames(viddata->getNFrames());
    vidctrl->setFrameRate(frameRate / dsTime);
    t_imgData->fileLoadCompleted(viddata->getNFrames(), viddata->getHeight(), viddata->getWidth());
    
    vidctrl->setEnabled(true);
    t_imgSettings->setEnabled(true);
    imview->setEnabled(true);
        
    updateContrastWidget(vidctrl->dff());
}

void Roivert::changeFrame(const qint32 frame)
{
    if (frame > 0 && frame <= viddata->getNFrames())
    { // frame is 1 indexed
        size_t f = frame;
        cv::Mat thisframe;

        // note that we should only be cloning if we have to (i.e. if there's some processing)...
        if (vidctrl->dff()) {
            thisframe = viddata->getFrameDff(f - 1).clone();
        }
        else {
            thisframe = viddata->getFrameRaw(f - 1).clone();
        }

        if (dispSettings.hasContrast(vidctrl->dff())) {
            cv::LUT(thisframe, dispSettings.getLut(vidctrl->dff()), thisframe);
        }
        QImage qimg(thisframe.data,
                    thisframe.cols,
                    thisframe.rows,
                    thisframe.step,
                    QImage::Format_Grayscale8);
        imview->setImage(qimg);
    }
 
}
void Roivert::frameRateChanged(double frameRate){
    vidctrl->setFrameRate(frameRate / viddata->getdsTime());
}

void Roivert::contrastChange(double min, double max, double gamma) {
    dispSettings.setContrast(vidctrl->dff(), min, max, gamma);
    vidctrl->forceUpdate();
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
    imview->setROIShape(ROIVert::ELLIPSE); // setting default here so it matches with checked

    actROIEllipse->setProperty("Shape", ROIVert::ELLIPSE);
    actROIPoly->setProperty("Shape", ROIVert::POLYGON);
    actROIRect->setProperty("Shape", ROIVert::RECTANGLE);

    actROIEllipse->setProperty("Mode", ROIVert::ADDROI);
    actROIPoly->setProperty("Mode", ROIVert::ADDROI);
    actROIRect->setProperty("Mode", ROIVert::ADDROI);
    actROISelect->setProperty("Mode", ROIVert::SELROI);

    actROIEllipse->setToolTip(tr("Draw ellipse ROIs"));
    actROIPoly->setToolTip(tr("Draw polygon ROIs"));
    actROIRect->setToolTip(tr("Draw rectangle ROIs"));
    actROISelect->setToolTip(tr("Select ROIs"));


    ui.mainToolBar->addActions(ROIGroup->actions());
    ui.mainToolBar->addSeparator();
    connect(ROIGroup, &QActionGroup::triggered, this, [=](QAction* act)
                {
                    ROIVert::MODE mode = (ROIVert::MODE)act->property("Mode").toInt();
                    imview->setMouseMode(mode);
                    if (mode == ROIVert::ADDROI) {
                        ROIVert::ROISHAPE shape = (ROIVert::ROISHAPE)act->property("Shape").toInt();
                        imview->setROIShape(shape);
                    }
                }
            );

    // add dockables...
    w_imgData->toggleViewAction()->setIcon(QIcon(":/icons/icons/t_ImgData.png"));
    ui.mainToolBar->addAction(w_imgData->toggleViewAction());
    w_imgSettings->toggleViewAction()->setIcon(QIcon(":/icons/icons/t_ImgSettings.png"));
    ui.mainToolBar->addAction(w_imgSettings->toggleViewAction());
    


    addToolBar(Qt::LeftToolBarArea, ui.mainToolBar);
}

void Roivert::updateContrastWidget(bool isDff) {
    // this sets histogram and contrast on the widget:
    float c[3];
    dispSettings.getRawContrast(isDff, &c[0]);
    t_imgSettings->setContrast(c[0],c[1],c[2]);

    // todo: consider taking same approach as I did with dispSettings, storing raw and dff in [0] and [1] and using bool to address...
    std::vector<float> hist; 
    isDff ? viddata->getHistogramDff(hist) : viddata->getHistogramRaw(hist);
    t_imgSettings->setHistogram(hist);
}