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
#include "qtextstream.h"
#include "qactiongroup.h"
#include "qmessagebox.h"
#include "qxmlstream.h"
#include "qfileinfo.h"
#include "qdir.h"
#include "qvalueaxis.h"

Roivert::Roivert(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    viddata = new VideoData(this);

    setStyleSheet("QMainWindow::separator { background-color: #bbb; width: 1px; height: 1px; }");

    QGridLayout* gridLayout = new QGridLayout(ui.centralWidget); // top level layout that makes everything stretch-to-fit
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

    w_imgSettings->setVisible(false);

    t_io = new tool::fileIO(this);
    w_io = new QDockWidget();
    w_io->setWidget(t_io);
    w_io->setWindowTitle("Import/Export");
    addDockWidget(Qt::RightDockWidgetArea, w_io);


    // Right Side:
    QWidget* rightLayoutWidget = new QWidget(ui.centralWidget);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightLayoutWidget);
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

    // Trace Computer:
    //tcompute = new TraceComputer;
    //tcompute->moveToThread(&traceThread);
    //traceThread.start();

    // Trace Viewer
    w_charts = new QDockWidget;
    tviewer = new TraceViewer(this);
    w_charts->setWidget(tviewer);
    addDockWidget(Qt::BottomDockWidgetArea, w_charts);


    connect(t_imgData, &tool::imgData::fileLoadRequested, this, &Roivert::loadVideo);
    connect(vidctrl, &VideoController::frameChanged, this, &Roivert::changeFrame);
    connect(viddata, &VideoData::loadProgress, t_imgData, &tool::imgData::setProgBar);
    connect(t_imgData, &tool::imgData::frameRateChanged, this, &Roivert::frameRateChanged);
    connect(vidctrl, &VideoController::dffToggle, this, &Roivert::updateContrastWidget);
    connect(t_imgSettings, &tool::imgSettings::imgSettingsChanged, this, &Roivert::imgSettingsChanged);
    connect(imview, &ImageROIViewer::roiEdited, this, &Roivert::updateTrace);
    connect(t_imgData, &tool::imgData::frameRateChanged, this, [=](double fr) {tviewer->setmaxtime(viddata->getNFrames() / fr); });
    connect(imview, &ImageROIViewer::roiSelectionChange, tviewer, &TraceViewer::setSelectedTrace);
    connect(tviewer, &TraceViewer::chartClicked, imview, &ImageROIViewer::setSelectedROI);
    connect(imview, &ImageROIViewer::toolfromkey, this, &Roivert::selecttoolfromkey);
    connect(imview, &ImageROIViewer::roiDeleted, tviewer, &TraceViewer::roideleted);
    connect(tviewer, &TraceViewer::deleteroi, imview, &ImageROIViewer::deleteROI);
    connect(t_io, &tool::fileIO::exportTraces, this, &Roivert::exportTraces);
    connect(t_io, &tool::fileIO::exportROIs, this, &Roivert::exportROIs);
    connect(t_io, &tool::fileIO::exportCharts, this, &Roivert::exportCharts);


    //connect(this, &Roivert::MupdateTrace, tcompute, &TraceComputer::update);
    //connect(this, &TraceComputer::traceComputed, tviewer, &TraceViewer::tracecomputed);

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
    vidctrl->setNFrames(viddata->getNFrames());
    vidctrl->setFrameRate(frameRate / dsTime);
    t_imgData->fileLoadCompleted(viddata->getNFrames(), viddata->getHeight(), viddata->getWidth());
    
    vidctrl->setEnabled(true);
    t_imgSettings->setEnabled(true);
    imview->setEnabled(true);
        
    //viddata->getNFrames() / frameRate;
    tviewer->setmaxtime(viddata->getNFrames() / frameRate);
    updateContrastWidget(vidctrl->dff());
}

void Roivert::changeFrame(const qint32 frame)
{
    if (frame > 0 && frame <= viddata->getNFrames())
    { // frame is 1 indexed
        size_t f = frame;
        cv::Mat thisframe;
        thisframe = viddata->get(vidctrl->dff(),dispSettings.getProjectionMode(),f-1);
        cv::Mat proc = dispSettings.getImage(thisframe, vidctrl->dff());
        QImage::Format fmt = dispSettings.useCmap() ? QImage::Format_BGR888 : QImage::Format_Grayscale8;
        QImage qimg(proc.data,proc.cols,proc.rows,proc.step,fmt);
        imview->setImage(qimg);
    }
 
}
void Roivert::frameRateChanged(double frameRate){
    vidctrl->setFrameRate(frameRate / viddata->getdsTime());
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
    w_charts->toggleViewAction()->setIcon(QIcon(":/icons/icons/t_Charts.png"));
    ui.mainToolBar->addAction(w_charts->toggleViewAction());



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


void Roivert::imgSettingsChanged(imgsettings settings) {
    dispSettings.setContrast(vidctrl->dff(), settings.contrastMin, settings.contrastMax, settings.contrastGamma);
    
    dispSettings.setProjectionMode(settings.projectionType);
    if (settings.projectionType > 0) { vidctrl->setStop(); }
    vidctrl->setEnabled(settings.projectionType == 0);

    dispSettings.setColormap(settings.cmap);
    dispSettings.setSmoothing(settings.smoothType, settings.smoothSize, settings.smoothSigma, settings.smoothSimgaI);
    
    vidctrl->forceUpdate();
}

void Roivert::updateTrace(int roiid)
{
    if (roiid < 1) { return; }
    size_t ind = (size_t)roiid - 1;

    roi *thisroi = imview->getRoi(ind);
    cv::Mat mask = thisroi->getMask();
    QRect r = thisroi->getBB();
    cv::Rect cvbb((size_t)r.x(), size_t(r.y()), (size_t)r.width(), (size_t)r.height());;

    if (ind >= traces.size() ) {
        traces.resize(ind + 1);
    }
    traces[ind] = viddata->calcTrace(cvbb, mask);
    tviewer->setTrace(roiid, traces[ind]);
}

void Roivert::selecttoolfromkey(int key) {
    int item = key - 49;
    if (item >= 0 && item < ui.mainToolBar->actions().size()) {
        QAction* act = ui.mainToolBar->actions()[item];
        //act->setChecked( ~act->isChecked());
        act->activate(QAction::Trigger);
    }
}

void Roivert::exportTraces(QString filename) {
    // all I need to do is overwrite the file and stuff it
    
    std::vector<float> t = tviewer->getTVec();
    std::vector<std::vector<float>> y = tviewer->getAllTraces();
    QMessageBox msg;
    msg.setWindowIcon(QIcon(":/icons/icons/GreenCrown.png"));
    
    if (t.empty()) {
        msg.setIcon(QMessageBox::Warning);
        msg.setText(tr("No traces to export."));
        msg.exec();
        return;
    }

    QFile file(filename);
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&file);    
        
        for (size_t i = 0; i < t.size(); i++) { // for each time
            out << t[i] << ",";
            for (size_t j = 0; j < y.size()-1; j++) { // for each trace
                out << y[j][i] << ",";
            }
            out << y[y.size() - 1][i];
            out << Qt::endl;
        }

        file.flush();
        file.close();
        msg.setText(tr("The traces have been exported."));
        msg.exec();
    }
    else {
        msg.setIcon(QMessageBox::Warning);
        msg.setText(tr("Could not write to this file, is it open?"));
        msg.exec();
    }
}

void Roivert::exportROIs(QString filename) {
    // all I need to do is overwrite the file and stuff it

    QMessageBox msg;
    msg.setWindowIcon(QIcon(":/icons/icons/GreenCrown.png"));

    QVector<QPair<ROIVert::ROISHAPE, QVector<QPoint>>> r = imview->getAllROIs();
    if (r.empty()) {
        msg.setIcon(QMessageBox::Warning);
        msg.setText(tr("No ROIs to export."));
        msg.exec();
        return;
    }

    QFile file(filename);
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        QXmlStreamWriter xml(&file);
        xml.setAutoFormatting(true);
        xml.writeStartDocument();

        std::map<ROIVert::ROISHAPE, QString> shapemap;
        shapemap.insert(std::make_pair(ROIVert::ROISHAPE::RECTANGLE, "rectangle"));
        shapemap.insert(std::make_pair(ROIVert::ROISHAPE::ELLIPSE, "ellipse"));
        shapemap.insert(std::make_pair(ROIVert::ROISHAPE::POLYGON, "polygon"));

        for each (QPair<ROIVert::ROISHAPE, QVector<QPoint>> roi in r)
        {
            xml.writeStartElement("ROI");
            xml.writeAttribute("shape",shapemap[roi.first]);
            for each (QPoint pt in roi.second)
            {
                xml.writeStartElement("Vertex");
                xml.writeAttribute("X", QString::number(pt.x()));
                xml.writeAttribute("Y", QString::number(pt.y()));
                xml.writeEndElement();
            }
            xml.writeEndElement();
        }

        file.flush();
        file.close();
        msg.setText(tr("The ROIs have been exported."));
        msg.exec();
    }
    else {
        msg.setIcon(QMessageBox::Warning);
        msg.setText(tr("Could not write to this file, is it open?"));
        msg.exec();
    }
}


void Roivert::exportCharts(QString filename) {
    // this is the hard one...we need to get a copy of each of the chartview pointers
    // going to reuse getTVec() and getAllTraces even though it's non-optimal (just unpacking and repacking the QPointFs...


    std::vector<float> t = tviewer->getTVec();
    std::vector<std::vector<float>> y = tviewer->getAllTraces();
    QMessageBox msg;
    msg.setWindowIcon(QIcon(":/icons/icons/GreenCrown.png"));

    if (t.empty()) {
        msg.setIcon(QMessageBox::Warning);
        msg.setText(tr("No traces to export."));
        msg.exec();
        return;
    }

    QFileInfo basefile(filename);
    QString basename(QDir(basefile.absolutePath()).filePath(basefile.completeBaseName()));


    QChartView* cv = new QChartView;
    QChart* ch = new QChart;
    QLineSeries* series = new QLineSeries;

    ch->addSeries(series);
    cv->setChart(ch);
    ch->createDefaultAxes();

    QValueAxis* xax = (QValueAxis*)ch->axes(Qt::Horizontal)[0];
    QValueAxis* yax = (QValueAxis*)ch->axes(Qt::Vertical)[0];
    xax->setTitleText("Time (s)");
    yax->setTitleText(QString::fromWCharArray(L"\x03B4\xD835\xDC53/\xD835\xDC53"));
    double xmin = *std::min_element(t.begin(), t.end());
    double xmax = *std::max_element(t.begin(), t.end());
    cv->setContentsMargins(0, 0, 0, 0);
    ch->layout()->setContentsMargins(0, 0, 0, 0);
    ch->legend()->hide();
    
    QPen pen(series->pen());
    pen.setWidth(3);
    series->setPen(pen);

    QFont font(ch->titleFont());
    font.setPointSize(18);
    ch->setTitleFont(font);
    
    font.setPointSize(16);
    xax->setTitleFont(font);
    yax->setTitleFont(font);

    font.setBold(false);
    font.setPointSize(14);
    xax->setLabelsFont(font);
    yax->setLabelsFont(font);


    for (size_t traces = 0; traces < y.size(); traces++) { // for each trace
        // make a series:
        QVector<QPointF> pts;
        pts.reserve(y[traces].size());
        for (size_t frames = 0; frames < y[traces].size(); frames++) {
            pts.push_back(QPointF(t[frames], y[traces][frames]));
        }
        series->replace(pts);

        double ymin = *std::min_element(y[traces].begin(), y[traces].end());
        double ymax = *std::max_element(y[traces].begin(), y[traces].end());
        yax->setMin(ymin);
        yax->setMax(ymax);
        xax->setMin(xmin);
        xax->setMax(xmax);
        ch->setTitle("ROI" + QString::number(traces + 1));

        cv->resize(1600, 600);
        cv->grab().toImage().save(basename + "_" + QString::number(traces + 1) + ".png");
    }
    delete(cv);// this should delete cv and series

}
