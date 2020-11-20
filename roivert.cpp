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
    viddata = new VideoData(this);

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
    QTime t;
    t.start();
    viddata->load(fileList, dsTime, dsSpace);
    
    qDebug() << "load time: " << t.elapsed()/1000. << "seconds";
    vidctrl->setNFrames(viddata->getNFrames());
    vidctrl->setFrameRate(frameRate / dsTime);

    /*
    if (fileList.empty()) { return; };

    size_t nfiles = fileList.size();
    size_t nframes = nfiles / dsTime;

    viddata->clear();
    viddata->reserve(nframes);

    vidmeta.dsSpace = dsSpace;
    vidmeta.dsTime = dsTime;

    // Load the first file to get size information, and initialize projections:
    std::string filename = fileList[0].toLocal8Bit().constData();
    cv::Mat image = cv::imread(filename, cv::IMREAD_GRAYSCALE);
    const cv::Size sz = image.size();
    vidmeta.init(image);

    for (size_t i = dsTime; i < nfiles; i += dsTime)
    {
        filename = fileList[i].toLocal8Bit().constData();
        image = cv::imread(filename, cv::IMREAD_GRAYSCALE);
        if (dsSpace > 1) {
            cv::resize(image.clone(), image, cv::Size(), 1. / dsSpace, 1. / dsSpace, cv::INTER_NEAREST); // do i need clone here?
        }

        if (sz == image.size()){
            viddata->push_back(image.clone());
            vidmeta.accum(image);
        }
    }

    vidctrl->setNFrames(viddata->size());
    vidctrl->setFrameRate(frameRate / dsTime);
    vidmeta.complete();

    // todo: pass something about success, i.e. if frames were skipped because they didn't match the size
    t_img->fileLoadCompleted(viddata->size(), sz.height, sz.width);
    */
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