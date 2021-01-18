// What a MESS!
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
#include "qsettings.h"
#include <QGraphicsLayout>


#include "TraceView.h"

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

    t_clrs = new tool::colors(this);
    w_colors = new QDockWidget();
    w_colors->setWidget(t_clrs);
    w_colors->setWindowTitle("Colors");
    w_colors->setObjectName("WColors");
    addDockWidget(Qt::RightDockWidgetArea, w_colors);
    w_colors->setVisible(false);

    
    
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

    // Trace Viewer
    w_charts = new QDockWidget;
    w_charts->setWindowTitle("Charts");
    traceview = new TraceView(&TraceData, this);
    w_charts->setObjectName("WCharts");
    w_charts->setWidget(traceview);
    addDockWidget(Qt::BottomDockWidgetArea, w_charts);


    connect(t_imgData, &tool::imgData::fileLoadRequested, this, &Roivert::loadVideo);
    connect(vidctrl, &VideoController::frameChanged, this, &Roivert::changeFrame);
    connect(viddata, &VideoData::loadProgress, t_imgData, &tool::imgData::setProgBar);
    connect(t_imgData, &tool::imgData::frameRateChanged, this, &Roivert::frameRateChanged);
    connect(vidctrl, &VideoController::dffToggle, this, &Roivert::updateContrastWidget);
    connect(t_imgSettings, &tool::imgSettings::imgSettingsChanged, this, &Roivert::imgSettingsChanged);
    connect(imview, &ImageROIViewer::roiEdited, this, &Roivert::updateTrace);
    connect(t_imgData, &tool::imgData::frameRateChanged, this, [&](double fr) {traceview->setTimeLimits(0, viddata->getNFrames() / fr); });

    connect(imview, &ImageROIViewer::toolfromkey, this, &Roivert::selecttoolfromkey);

    connect(traceview, &TraceView::roiDeleted, imview, &ImageROIViewer::deleteROI);
    connect(traceview, &TraceView::traceSelected, imview, &ImageROIViewer::setSelectedROI);
    connect(imview, &ImageROIViewer::roiDeleted, this, [=](size_t roiid) {
        // remove the trace. note that removing a row from a mat is hard
        cv::Mat newTraceData = cv::Mat( TraceData.size().height-1, TraceData.size().width, TraceData.type());

        if (TraceData.size().height >1 ) {
            size_t row = roiid - 1;
            if (row > 0) {
                cv::Rect rect(0, 0, TraceData.size().width, row);
                TraceData(rect).copyTo(newTraceData(rect));
            }
            if (row < TraceData.size().height - 1) {
                cv::Rect rect1(0, row + 1, TraceData.size().width, TraceData.size().height - row - 1);
                cv::Rect rect2(0, row, TraceData.size().width, TraceData.size().height - row - 1);
                TraceData(rect1).copyTo(newTraceData(rect2));
            }
        }
        TraceData = newTraceData;
        
        // update
        traceview->removeTrace(roiid);
    });
    connect(imview, &ImageROIViewer::roiSelectionChange, this, [=](size_t oldid, size_t roiid) {
        traceview->select(roiid);
    });


    connect(t_io, &tool::fileIO::exportTraces, this, &Roivert::exportTraces);
    connect(t_io, &tool::fileIO::exportCharts, this, &Roivert::exportCharts);
    connect(t_io, &tool::fileIO::exportROIs, this, &Roivert::exportROIs); 
    connect(t_io, &tool::fileIO::importROIs, this, &Roivert::importROIs);
    
    connect(t_clrs, &tool::colors::setSelectedColor, imview, &ImageROIViewer::setSelectedColor);
    connect(t_clrs, &tool::colors::setUnselectedColor, imview, &ImageROIViewer::setUnselectedColor);

    imview->setMouseMode(ROIVert::ADDROI);
    imview->setROIShape(ROIVert::RECTANGLE);

    makeToolbar();

    // Action that resets window state:
    QAction* actResetLayout = new QAction(tr("Reset Layout"));
    actResetLayout->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_R));
    connect(actResetLayout, &QAction::triggered, this, &Roivert::resetLayout);
    addAction(actResetLayout);

    setWindowIcon(QIcon(":/icons/icons/GreenCrown.png"));
    resize(800, 900);

    restoreSettings();
}

void Roivert::loadVideo(const QStringList fileList, const double frameRate, const int dsTime, const int dsSpace)
{
    // confirm load if rois exist:
    if (imview->getNROIs() > 0) {
        // rois exist:
        QMessageBox msg;
        msg.setText(tr("Existing ROIs will be removed when loading a new file, continue?"));
        msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        const int ret = msg.exec();
        if (ret == QMessageBox::Cancel) {
            return;
        }
        while (imview->getNROIs() > 0) {
            imview->deleteROI(1);
        }
    }

    viddata->load(fileList, dsTime, dsSpace);
    t_imgData->setProgBar(-1);
    vidctrl->setNFrames(viddata->getNFrames());
    vidctrl->setFrameRate(frameRate / dsTime);
    t_imgData->fileLoadCompleted(viddata->getNFrames(), viddata->getHeight(), viddata->getWidth());
    
    vidctrl->setEnabled(true);
    t_imgSettings->setEnabled(true);
    imview->setEnabled(true);
    t_io->setEnabled(true);
        
    //viddata->getNFrames() / frameRate;
    traceview->setTimeLimits(0, viddata->getNFrames() / frameRate);

    updateContrastWidget(vidctrl->dff());
}

void Roivert::changeFrame(const size_t frame)
{
    if (frame > 0 && frame <= viddata->getNFrames())
    { 
        cv::Mat thisframe;
        thisframe = viddata->get(vidctrl->dff(),dispSettings.getProjectionMode(),frame-1);
        cv::Mat proc = dispSettings.getImage(thisframe, vidctrl->dff());
        // todo: consider moving fmt to dispSettings
        const QImage::Format fmt = dispSettings.useCmap() ? QImage::Format_BGR888 : QImage::Format_Grayscale8;
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
    connect(ROIGroup, &QActionGroup::triggered, this, [&](QAction* act)
                {
                    ROIVert::MODE mode = static_cast<ROIVert::MODE>(act->property("Mode").toInt());
                    imview->setMouseMode(mode);
                    if (mode == ROIVert::ADDROI) {
                        ROIVert::ROISHAPE shape = static_cast<ROIVert::ROISHAPE>(act->property("Shape").toInt());
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
    w_io->toggleViewAction()->setIcon(QIcon(":/icons/icons/t_io.png"));
    ui.mainToolBar->addAction(w_io->toggleViewAction());
    w_colors->toggleViewAction()->setIcon(QIcon(":/icons/icons/t_Colors.png"));
    ui.mainToolBar->addAction(w_colors->toggleViewAction());

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
    
    dispSettings.setContrast(vidctrl->dff(), settings.Contrast);
    
    dispSettings.setProjectionMode(settings.projectionType);
    if (settings.projectionType > 0) { vidctrl->setStop(); }
    vidctrl->setEnabled(settings.projectionType == 0);

    dispSettings.setColormap(settings.cmap);
    dispSettings.setSmoothing(settings.Smoothing);
    
    vidctrl->forceUpdate();
}

void Roivert::updateTrace(int roiid)
{
    if (roiid < 1) { return; }
    const size_t ind = static_cast<size_t>(roiid - 1);

    roi *thisroi = imview->getRoi(ind);
    if (thisroi) {
        cv::Mat mask = thisroi->getMask();
        const cv::Rect cvbb(ROIVert::QRect2CVRect(thisroi->getBB()));
        // new plotter
        viddata->computeTrace(cvbb, mask, roiid, TraceData);
        traceview->updateTraces(roiid,false);
    }

    
}

void Roivert::selecttoolfromkey(int key) {
    const int item = key - Qt::Key_1;
    if (item >= 0 && item < ui.mainToolBar->actions().size()) {
        QAction* act = ui.mainToolBar->actions()[item];
        act->activate(QAction::Trigger);
    }
}

void Roivert::exportTraces(QString filename, bool doHeader, bool doTimeCol) {
    // exportTraces needs rewrite, should be easier now (as it's a mat!)



    // todo:  I scraped these traces out of tviewer, but this has traces in it...why am I getting them from the viewer?
    /*
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

        if (doHeader) {
            if (doTimeCol) {
                out << "\"Time\",";
            }
            for (size_t j = 0; j < y.size() - 1; j++) {
                out << "\"ROI " + QString::number(j+1) + "\",";
            }
            out << "\"ROI " + QString::number(y.size()) + "\"";
            out << Qt::endl;
        }

        
        for (size_t i = 0; i < t.size(); i++) { // for each time
            if (doTimeCol) {
                out << t[i] << ",";
            }
            
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
    */
}

void Roivert::exportROIs(QString filename) {
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
        
        const int ds = viddata->getdsSpace();

        xml.writeStartElement("ROIs");

        for (auto roi : r)
        {
            xml.writeStartElement("ROI");
            xml.writeAttribute("shape",shapemap[roi.first]);
            for (auto pt : roi.second)
            {
                xml.writeStartElement("Vertex");
                xml.writeAttribute("X", QString::number(pt.x()*ds));
                xml.writeAttribute("Y", QString::number(pt.y()*ds));
                xml.writeEndElement();
            }
            xml.writeEndElement();
        }
        xml.writeEndElement();

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

void Roivert::importROIs(QString filename) {
    QMessageBox msg;
    msg.setWindowIcon(QIcon(":/icons/icons/GreenCrown.png"));

    QFile file(filename);

    //std::vector<roi*> rois;
    std::vector<std::unique_ptr<roi>> rois;

    if (file.open(QFile::ReadOnly)) {
        std::map<QString, ROIVert::ROISHAPE> shapemap;
        shapemap.insert(std::make_pair("rectangle", ROIVert::ROISHAPE::RECTANGLE));
        shapemap.insert(std::make_pair("ellipse", ROIVert::ROISHAPE::ELLIPSE));
        shapemap.insert(std::make_pair("polygon", ROIVert::ROISHAPE::POLYGON));

        QXmlStreamReader xml(&file);
        const int ds = viddata->getdsSpace();

        ROIVert::ROISHAPE type_e;
        QVector<QPoint> verts;
        while (!xml.atEnd()) {
            const QXmlStreamReader::TokenType tkn =  xml.readNext();
            if (xml.isStartElement() && xml.name()=="ROI" && xml.attributes().size()==1) {
                QString type_s(xml.attributes()[0].value().toString());
                type_e = shapemap[type_s];
                switch (type_e)
                {
                case ROIVert::RECTANGLE:
                    rois.push_back(std::make_unique<roi_rect>());
                    verts.clear();
                    break;
                case ROIVert::ELLIPSE:
                    rois.push_back(std::make_unique<roi_ellipse>());
                    verts.clear();
                    break;
                case ROIVert::POLYGON:
                    rois.push_back(std::make_unique<roi_polygon>());
                    verts.clear();
                    break;
                default:
                    break;
                }
            }
            if (xml.isStartElement() && xml.name() == "Vertex" && xml.attributes().size()==2) {
                const QPoint thispoint(QPoint(xml.attributes()[0].value().toInt() / ds, xml.attributes()[1].value().toInt()/ds));
                if (thispoint.x() < 0 || thispoint.x() > viddata->getWidth() || thispoint.y() < 0 || thispoint.y() > viddata->getHeight()) {
                    // TODO: factor out exceptions and handle outside.
                    msg.setIcon(QMessageBox::Critical);
                    msg.setText(tr("A vertex in the ROI file exceeds the boundaries of the current image."));
                    msg.exec();
                    return;
                }
                verts.push_back(thispoint);
            }
            if (xml.isEndElement() && xml.name()=="ROI") {
                rois.back()->setVertices(verts);
            }
        }
        if (xml.hasError()) {
            msg.setIcon(QMessageBox::Critical);
            msg.setText(tr("Error reading the ROI file."));
            msg.exec();
            return;
        }
    }
    else {
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Error reading the ROI file."));
        msg.exec();
        return;
    }
    
    // 
    imview->importROIs(rois);

    // clear rois vector to free the memory
    rois.clear();

}

void Roivert::exportCharts(QString filename, bool doTitle, int width, int height) {
    // chart export currently disabled!

}

void Roivert::closeEvent(QCloseEvent* event) {
    QSettings settings("Neuroph", "ROIVert");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());

    // restore colors
    QVector<QPair<QString, QColor>> clrs = t_clrs->getColors();

    for each (QPair<QString,QColor> pair in clrs)
    {
        settings.setValue("color/" + pair.first, pair.second);
    }
    QMainWindow::closeEvent(event);
}

void Roivert::restoreSettings()
{
    QSettings settings("Neuroph", "ROIVert");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    QVector<QPair<QString, QColor>> clrs;
    settings.beginGroup("color");
    QStringList keys = settings.childKeys();

    for each (QString key in keys)
    {
        QPair<QString, QColor> clr;
        clr.first = key;
        clr.second = settings.value(key).value<QColor>();
        clrs.push_back(clr);
    }

    t_clrs->setColors(clrs);
}
void Roivert::resetLayout() {
    // Dock all dockables in default position
    addDockWidget(Qt::BottomDockWidgetArea, w_charts);
    addDockWidget(Qt::RightDockWidgetArea, w_imgData);
    addDockWidget(Qt::RightDockWidgetArea, w_imgSettings);
    addDockWidget(Qt::RightDockWidgetArea, w_io);
    addDockWidget(Qt::RightDockWidgetArea, w_colors);

    w_charts->setFloating(false);
    w_imgData->setFloating(false);
    w_imgSettings->setFloating(false);
    w_io->setFloating(false);
    w_colors->setFloating(false);

    // Set dockables to visible off (except file loader)
    w_charts->setVisible(true);
    w_imgData->setVisible(true);

    w_imgSettings->setVisible(false);
    w_io->setVisible(false);
    w_colors->setVisible(false);

    // Put toolbar at left
    addToolBar(Qt::LeftToolBarArea, ui.mainToolBar);

    qApp->processEvents(QEventLoop::AllEvents);

    // Set window size
    resize(800, 900);
}