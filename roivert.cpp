#include "roivert.h"
#include "qboxlayout.h"
#include "qsplitter.h"
#include "qpushbutton.h"
#include "qtoolbox.h"
#include "qformlayout.h"
#include "qcheckbox.h"
#include "qspinbox.h"
#include "qdockwidget.h"

Roivert::Roivert(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

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
    // loadVideo slot : this is the main loading loop
    size_t nfiles = fileList.size();
    size_t nframes = nfiles / dsTime;

    viddata->clear();
    viddata->reserve(nframes);

    for (size_t i = 0; i < nfiles; i += dsTime)
    {

        std::string filename = fileList[i].toLocal8Bit().constData();
        cv::Mat image = cv::imread(filename, cv::IMREAD_GRAYSCALE);

        // downsample in space here..
        viddata->push_back(image.clone());

        // accumulate stuff
    }
    vidctrl->setNFrames(viddata->size());
    vidctrl->setFrameRate(frameRate / dsTime);
}

void Roivert::changeFrame(const qint32 frame)
{
    if (frame > 0 && frame <= viddata->size())
    { // frame is 1 indexed
        QTime t;

        cv::Mat thisframe = viddata->at((size_t)frame - 1);
        QImage qimg(thisframe.data,
                    thisframe.cols,
                    thisframe.rows,
                    thisframe.step,
                    QImage::Format_Grayscale8);
        imview->setImage(qimg);
    }
}