#include "FileIO.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QObject>
#include <QString>

#include "widgets/TraceChartWidget.h"

static QString tr(const char* sourceText, const char* disambiguation = nullptr, int n = -1)
{
    return QObject::tr(sourceText, disambiguation, n);
}
static bool validateROIsJson(QJsonObject json, QSize maxsize)
{
    // this is just looking for things that are likely to cause a crash...

    auto jrois{ json["ROIs"].toArray() };
    for (const auto& jroi : jrois)
    {
        auto roi{ jroi.toObject() };
        auto shape{ roi["shape"].toObject() };

        QJsonArray jrgb = shape["RGB"].toArray();
        if (jrgb.size() != 3)
        {
            return false;
        }

        auto type{ shape["type"].toInt() };
        if (type > 2)
        {
            return false;
        }
        auto jverts{ shape["verts"].toArray() };
        if (jverts.size() < 2)
        {
            return false;
        }
        for (const auto& vert : jverts)
        {
            auto arr{ vert.toArray() };
            if (arr.size() != 2)
            {
                return false;
            }
            if (arr.at(0).toInt() < 0 || arr.at(0).toInt() > maxsize.width() ||
                arr.at(1).toInt() < 0 || arr.at(1).toInt() > maxsize.height())
            {
                return false;
            }
        }
    }
    return true;
}

struct FileIO::pimpl
{
    ROIs* rois{ nullptr };
    TraceViewWidget* traceview{ nullptr };
    VideoData* videodata{ nullptr };

    QSize getMaxSize() const noexcept
    {
        QSize ret;
        if (videodata)
        {
            ret.setWidth(videodata->getWidth() * videodata->getdsSpace());
            ret.setHeight(videodata->getHeight() * videodata->getdsSpace());
        }
        return ret;
    }
};

FileIO::FileIO(ROIs* rois, TraceViewWidget* traceview, VideoData* videodata) : impl(std::make_unique<pimpl>())
{
    impl->rois = rois;
    impl->traceview = traceview;
    impl->videodata = videodata;
}
FileIO::~FileIO() = default;

void FileIO::exportTraces(QString filename, bool includeheader, bool includetime) const
{
    QMessageBox msg;
    msg.setWindowIcon(QIcon(":/icons/GreenCrown.png"));
    msg.setIcon(QMessageBox::Warning);
    if (impl->rois == nullptr || impl->rois->size() < 1)
    {
        msg.setText(tr("No traces to export."));
        msg.exec();
        return;
    }

    size_t ntraces = impl->rois->size();
    std::vector<std::vector<float>> traces(ntraces);
    for (size_t i = 0; i < ntraces; ++i) {
        traces[i] = (*impl->rois)[i].Trace->getTrace();
    }

    // need time minmax
    const auto tmax = impl->videodata->getTMax();
    const auto nframes = impl->videodata->getNFrames();

    QFile file(filename);
    const bool openable = file.open(QFile::WriteOnly | QFile::Truncate);
    if (!openable)
    {
        msg.setText(tr("Could not write to this file, is it open in another program?"));
        msg.exec();
        return;
    }

    QTextStream out(&file);

    if (includeheader)
    {
        if (includetime)
        {
            out << "\"Time\",";
        }
        for (size_t i = 0; i < impl->rois->size() - 1; ++i)
        {
            out << "\"ROI " + QString::number(i + 1) + "\",";
        }
        out << "\"ROI " + QString::number(ntraces) + "\"";
        out << Qt::endl;
    }

    for (size_t t = 0; t < nframes; ++t)
    {
        if (includetime)
        {
            const float tt = tmax * static_cast<float>(t) / static_cast<float>(nframes);
            out << tt << ",";
        }
        for (size_t trace = 0; trace < ntraces - 1; ++trace)
        {
            out << traces[trace][t] << ",";
        }
        out << traces[ntraces - 1][t];
        out << Qt::endl;
    }
    file.flush();
    file.close();
}
void FileIO::importROIs(QString filename) const
{
    QMessageBox msg;
    msg.setWindowIcon(QIcon(":/icons/GreenCrown.png"));
    msg.setIcon(QMessageBox::Warning);

    QFile file(filename);
    const bool openable = file.open(QFile::ReadOnly);
    if (!openable || impl->rois == nullptr)
    {
        msg.setText(tr("Error reading the ROI file."));
        msg.exec();
        return;
    }

    QByteArray saveData = file.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    // can the doc be validated first?
    auto json = loadDoc.object();

    const bool isok = validateROIsJson(json, impl->getMaxSize());
    if (!isok)
    {
        msg.setText(tr("Error reading the ROI file."));
        msg.exec();
        return;
    }

    impl->rois->deleteAllROIs();
    impl->rois->read(json);
}
void FileIO::exportROIs(QString filename) const
{
    QMessageBox msg;
    msg.setWindowIcon(QIcon(":/icons/GreenCrown.png"));
    msg.setIcon(QMessageBox::Warning);
    if (impl->rois == nullptr || impl->rois->size() < 1)
    {
        msg.setText(tr("No traces to export."));
        msg.exec();
        return;
    }
    QFile file(filename);
    const bool openable = file.open(QFile::WriteOnly | QFile::Truncate);
    if (!openable)
    {
        msg.setText(tr("Could not write to this file, is it open in another program?"));
        msg.exec();
        return;
    }

    auto json = QJsonObject();
    json["version"] = ROIVERTVERSION;

    impl->rois->write(json);
    QJsonDocument saveDoc(json);
    file.write(saveDoc.toJson());
}

void FileIO::exportCharts(QString filename, int width, int height, int quality, bool ridge)
{
    if (ridge)
    {
        impl->traceview->getRidgeChart().saveAsImage(filename, width, height, quality);
    }
    else
    {
        if (impl->rois == nullptr || impl->rois->size() < 1)
        {
            QMessageBox msg;
            msg.setWindowIcon(QIcon(":/icons/GreenCrown.png"));
            msg.setIcon(QMessageBox::Warning);
            msg.setText(tr("No charts to export."));
            msg.exec();
            return;
        }

        const QFileInfo basefile(filename);
        const QString basename(QDir(basefile.absolutePath()).filePath(basefile.completeBaseName()));
        const QString suffix = basefile.completeSuffix();
        for (size_t i = 0; i < impl->rois->size(); ++i) {
            const QString filename(basename + "_" + QString::number(i + 1) + "." + suffix);
            (*impl->rois)[i].Trace->getTraceChart()->saveAsImage(filename, width, height, quality);
        }
    }
}