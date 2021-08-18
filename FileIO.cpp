#include "FileIO.h"
#include <QDebug>
#include <QMessageBox>
#include <QIcon>
#include <QString>
#include <QObject>
#include <QFile>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

static QString tr(const char* sourceText, const char* disambiguation = nullptr, int n = -1) {
    return QObject::tr(sourceText, disambiguation, n);
}

struct FileIO::pimpl {
    ROIs* rois{ nullptr };
    TraceView* traceview{ nullptr };
    VideoData* videodata{ nullptr };
};

FileIO::FileIO(ROIs* rois, TraceView* traceview, VideoData* videodata) {
    impl->rois = rois;
    impl->traceview = traceview;
    impl->videodata = videodata;
}
FileIO::~FileIO() = default;

void FileIO::exportTraces(QString filename, bool includeheader, bool includetime) const {
    // exportTraces needs rewrite, should be easier now (as it's a mat!)
    QMessageBox msg;
    msg.setWindowIcon(QIcon(":/icons/icons/GreenCrown.png"));
    msg.setIcon(QMessageBox::Warning);
    
    auto ntraces = impl->rois->getNROIs();
    if (impl->rois == nullptr || ntraces < 1) {
        msg.setText(tr("No traces to export."));
        msg.exec();
        return;
    }

    auto inds{ std::vector<size_t>(impl->rois->getNROIs()) };
    std::iota(inds.begin(), inds.end(), 0);
    auto traces{ impl->rois->getTraces(inds) };

    // need time minmax
    const auto tmax = impl->videodata->getTMax();
    auto nframes = impl->videodata->getNFrames();

    QFile file(filename);
    bool openable = file.open(QFile::WriteOnly | QFile::Truncate);
    if (!openable) {
        msg.setText(tr("Could not write to this file, is it open in another program?"));
        msg.exec();
        return;
    }
            
    QTextStream out(&file);
    
    if (includeheader) {
        if (includetime) {
            out << "\"Time\",";
        }
        for (size_t i = 0; i < ntraces - 1; ++i) {
            out << "\"ROI " + QString::number(i + 1) + "\",";
        }
        out << "\"ROI " + QString::number(ntraces) + "\"";
        out << Qt::endl;
    }

    for (size_t t = 0; t < nframes; ++t) {
        if (includetime) {
            const float tt = tmax * static_cast<float>(t) / static_cast<float>(nframes);
            out << tt << ",";
        }
        for (size_t trace = 0; trace < ntraces - 1; ++trace){
            out << traces[trace][t] << ",";
        }
        out << traces[ntraces - 1][t];
        out << Qt::endl;
    }
    file.flush();
    file.close();
}
void FileIO::importROIs(QString filename) const {
    QMessageBox msg;
    msg.setWindowIcon(QIcon(":/icons/icons/GreenCrown.png"));
    msg.setIcon(QMessageBox::Warning);
    
    qDebug() << "IMPORT ROIS";
    QFile file(filename);
    bool openable = file.open(QFile::ReadOnly);
    if (!openable || impl->rois == nullptr) {
        msg.setText(tr("Error reading the ROI file."));
        msg.exec();
        return;
    }
    
    QByteArray saveData = file.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    
    // can the doc be validated first?
    impl->rois->deleteAllROIs();
    impl->rois->read(loadDoc.object());
}
void FileIO::exportROIs(QString filename) const {

    QMessageBox msg;
    msg.setWindowIcon(QIcon(":/icons/icons/GreenCrown.png"));
    msg.setIcon(QMessageBox::Warning);
    auto ntraces = impl->rois->getNROIs();
    if (impl->rois == nullptr || ntraces < 1) {
        msg.setText(tr("No traces to export."));
        msg.exec();
        return;
    }
    // todo: file export/import needs to somehow use pixel subset...
    
    QFile file(filename);
    bool openable = file.open(QFile::WriteOnly | QFile::Truncate);
    if (!openable) {
        msg.setText(tr("Could not write to this file, is it open in another program?"));
        msg.exec();
        return;
    }

    auto json = QJsonObject();
    impl->rois->write(json);
    QJsonDocument saveDoc(json);
    file.write(saveDoc.toJson());

}
void FileIO::exportLineCharts(int width, int height, int quality) const {}


    