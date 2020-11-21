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

Roivert::Roivert(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    viddata = new VideoData(this);

    setStyleSheet("QMainWindow::separator { background-color: #bbb; width: 1px; height: 1px; }");
    
    QGridLayout *gridLayout = new QGridLayout(ui.centralWidget); // top level layout that makes everything stretch-to-fit
    setDockNestingEnabled(true);

    t_imgData = new tool::imgData(this);
    QDockWidget* testwidg = new QDockWidget();
    testwidg->setWidget(t_imgData);
    testwidg->setWindowTitle("Image Data");
    addDockWidget(Qt::LeftDockWidgetArea, testwidg);

    t_imgSettings = new tool::imgSettings(this);
    QDockWidget* testwidg2 = new QDockWidget();
    testwidg2->setWidget(t_imgSettings);
    testwidg2->setWindowTitle("Image Settings");
    addDockWidget(Qt::LeftDockWidgetArea, testwidg2);

    
    // Right Side:
    QWidget *rightLayoutWidget = new QWidget(ui.centralWidget);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightLayoutWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    // Image Viewer:
    imview = new ImageROIViewer(rightLayoutWidget);
    rightLayout->addWidget(imview);

    // Contols:
    vidctrl = new VideoController(rightLayoutWidget);
    rightLayout->addWidget(vidctrl);
    gridLayout->addWidget(rightLayoutWidget);

    connect(t_imgData, &tool::imgData::fileLoadRequested, this, &Roivert::loadVideo);
    connect(vidctrl, &VideoController::frameChanged, this, &Roivert::changeFrame);
    connect(viddata, &VideoData::loadProgress, t_imgData, &tool::imgData::setProgBar);
    connect(t_imgData, &tool::imgData::frameRateChanged, this, &Roivert::frameRateChanged);
    connect(vidctrl, &VideoController::dffToggle, t_imgSettings, &tool::imgSettings::setDffVisible);
    connect(t_imgSettings, &tool::imgSettings::contrastChanged, this, &Roivert::contrastChange);

    QImage testimage("C:\\Users\\dbulk\\OneDrive\\Documents\\qtprojects\\Roivert\\greenking.png");

    imview->setImage(testimage);
    imview->setMouseMode(ROIVert::ADDROI);
    imview->setROIShape(ROIVert::RECTANGLE);

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

    std::vector<float> rawhist;viddata->getHistogramRaw(rawhist);
    std::vector<float> dffhist; viddata->getHistogramDff(dffhist);

    t_imgSettings->setRawHistogram(rawhist);
    t_imgSettings->setDffHistogram(dffhist);
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
