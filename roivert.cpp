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

Roivert::Roivert(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    viddata = new VideoData(this);

    setStyleSheet("QMainWindow::separator { background-color: #bbb; width: 1px; height: 1px; }");
    
    QGridLayout *gridLayout = new QGridLayout(ui.centralWidget); // top level layout that makes everything stretch-to-fit
    setDockNestingEnabled(true);

    t_img = new tool::imgData(this);
    QDockWidget *testwidg = new QDockWidget();
    testwidg->setWidget(t_img);
    testwidg->setWindowTitle("Image Data");
    addDockWidget(Qt::LeftDockWidgetArea, testwidg);

    
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

    connect(t_img, &tool::imgData::fileLoadRequested, this, &Roivert::loadVideo);
    connect(vidctrl, &VideoController::frameChanged, this, &Roivert::changeFrame);
    connect(viddata, &VideoData::loadProgress, t_img, &tool::imgData::setProgBar);
    connect(t_img, &tool::imgData::frameRateChanged, this, &Roivert::frameRateChanged);


    // testcode:
    //QImage testimage("C://Users//dbulk//OneDrive//Documents//qtprojects//Roivert//testimage.JPG");
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
    t_img->setProgBar(-1);
    qDebug() << "load time: " << t.elapsed()/1000. << "seconds";
    vidctrl->setNFrames(viddata->getNFrames());
    vidctrl->setFrameRate(frameRate / dsTime);

    t_img->fileLoadCompleted(viddata->getNFrames(), viddata->getHeight(), viddata->getWidth());
}

void Roivert::changeFrame(const qint32 frame)
{
    if (frame > 0 && frame <= viddata->getNFrames())
    { // frame is 1 indexed
        size_t f = frame;

        cv::Mat thisframe;
        if (vidctrl->dff()) {
            thisframe = viddata->getFrameDff(f - 1);
        }
        else {
            thisframe = viddata->getFrameRaw(f - 1);
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