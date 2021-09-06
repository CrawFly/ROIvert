#include "dockwidgets/ImageSettingsWidget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

#include "DisplaySettings.h"
#include "widgets/ContrastWidget.h"
#include "widgets/ProjectionPickWidget.h"
#include "widgets/ColormapPickWidget.h"
#include "widgets/SmoothingPickWidget.h"

static void addVSep(QVBoxLayout *lay)
{
    if (lay)
    {
        QFrame *line = new QFrame;
        line->setFrameStyle(QFrame::HLine);
        lay->addWidget(line);
    }
}

struct ImageSettingsWidget::pimpl
{
    QWidget *contents;
    DisplaySettings* dispsettings{ nullptr }; //*** communication is currently through signal/slot, but dispsettings ptr is used for save/load/reset

    ContrastWidget *contrast = new ContrastWidget;
    ProjectionPickWidget *projection = new ProjectionPickWidget;
    ColormapPickWidget *colormap = new ColormapPickWidget;
    SmoothingPickWidget *Wsmoothing = new SmoothingPickWidget;
    QPushButton *dffToggle = new QPushButton;

    void init()
    {
        dffToggle->setText("df/f");
        dffToggle->setCheckable(true);
        dffToggle->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        dffToggle->setToolTip(tr("Toggle df/f mode, each pixel will be normalized by subtracting, and then dividing by, the average for that pixel over time"));

        contrast->setMaximumHeight(300);
        contrast->setMaximumWidth(300);
        projection->setMaximumWidth(300);
        colormap->setMaximumWidth(300);
        Wsmoothing->setMaximumWidth(300);
    }

    void layout(QVBoxLayout *topLay)
    {
        topLay->setAlignment(Qt::AlignTop);
        topLay->setContentsMargins(10, 0, 10, 10);

        topLay->addWidget(new QLabel(tr("Contrast:")));
        topLay->addWidget(contrast);
        addVSep(topLay);

        {
            auto lay = new QHBoxLayout;
            lay->addWidget(new QLabel(tr("Projection:")));
            lay->addSpacing(100);
            lay->addWidget(dffToggle);
            lay->addStretch(1);
            topLay->addLayout(lay);
        }

        topLay->addWidget(projection);
        addVSep(topLay);
        topLay->addWidget(new QLabel(tr("Colormap:")));
        topLay->addWidget(colormap);
        addVSep(topLay);
        topLay->addWidget(new QLabel(tr("Smoothing:")));
        topLay->addWidget(Wsmoothing);
        topLay->addStretch(1);
    }

    ROIVert::imgsettings updateSettings()
    {
        // this is a centralized location for updating settings and emitting a signal with the whole payload:
        ROIVert::imgsettings payload;
        payload.Contrast = contrast->getContrast();
        payload.projectionType = projection->getProjection();
        payload.cmap = colormap->getColormap();
        payload.Smoothing = Wsmoothing->getSmoothing();
        
        return payload;
    }
};

ImageSettingsWidget::ImageSettingsWidget(QWidget *parent, DisplaySettings* dispsettings) : DockWidgetWithSettings(parent)
{
    impl->dispsettings = dispsettings;
    impl->contents = new QWidget;
    toplay.addWidget(impl->contents);

    QVBoxLayout *lay = new QVBoxLayout;
    impl->contents->setLayout(lay);

    impl->init();
    impl->layout(lay);

    auto lam = [=]
    {
        auto newsettings = impl->updateSettings();
        emit imgSettingsChanged(newsettings);
    };

    connect(impl->contrast, &ContrastWidget::contrastChanged, lam);
    connect(impl->projection, &ProjectionPickWidget::projectionChanged, lam);
    connect(impl->colormap, &ColormapPickWidget::colormapChanged, lam);
    connect(impl->Wsmoothing, &SmoothingPickWidget::smoothingChanged, lam);
    connect(impl->dffToggle, &QPushButton::clicked, this, &ImageSettingsWidget::dffToggled);
}
ImageSettingsWidget::~ImageSettingsWidget() = default;

void ImageSettingsWidget::setHistogram(std::vector<float> &data)
{
    // todo: consider whether this should come in as a QVector to avoid the conversion (or vice versa)
    impl->contrast->setHistogram(QVector<float>::fromStdVector(data));
}
void ImageSettingsWidget::setContrast(ROIVert::contrast c)
{
    impl->contrast->setContrast(c);
}
void ImageSettingsWidget::setContentsEnabled(bool onoff)
{
    impl->contents->setEnabled(onoff);
}

void ImageSettingsWidget::dffToggle(bool onoff)
{
    impl->dffToggle->setChecked(onoff);
}

void ImageSettingsWidget::saveSettings(QSettings& settings) const {

    settings.beginGroup("ImageSettings");
    settings.setValue("dorestore", getSettingsStorage());
    if (getSettingsStorage()) {
        auto rawContrast = impl->dispsettings->getContrast(false);
        auto dffContrast = impl->dispsettings->getContrast(true);
        
        settings.setValue("rawCont0", std::get<0>(rawContrast));
        settings.setValue("rawCont1", std::get<1>(rawContrast));
        settings.setValue("rawCont2", std::get<2>(rawContrast));
        settings.setValue("dffCont0", std::get<0>(dffContrast));
        settings.setValue("dffCont1", std::get<1>(dffContrast));
        settings.setValue("dffCont2", std::get<2>(dffContrast));
        
        auto currSettings = impl->updateSettings();
        settings.setValue("cmap", currSettings.cmap);
        settings.setValue("proj", currSettings.projectionType);

        settings.setValue("smoothing0",std::get<0>(currSettings.Smoothing));
        settings.setValue("smoothing1",std::get<1>(currSettings.Smoothing));
        settings.setValue("smoothing2",std::get<2>(currSettings.Smoothing));
        settings.setValue("smoothing3",std::get<3>(currSettings.Smoothing));
    }
    settings.endGroup();
}
void ImageSettingsWidget::restoreSettings(QSettings& settings) {
    settings.beginGroup("ImageSettings");
    setSettingsStorage(settings.value("dorestore", true).toBool());


    if (getSettingsStorage()) {
        //setContrast({ settings.value("cont0", 0.).toFloat(), settings.value("cont1", 1.).toFloat(),settings.value("cont2", 1.).toFloat() });
        impl->dispsettings->setContrast(false, { settings.value("rawCont0", 0.).toFloat(), settings.value("rawCont1", 1.).toFloat(),settings.value("rawCont2", 1.).toFloat() });
        impl->dispsettings->setContrast(true, { settings.value("dffCont0", 0.).toFloat(), settings.value("dffCont1", 1.).toFloat(),settings.value("dffCont2", 1.).toFloat() });
        setContrast(impl->dispsettings->getContrast(false));

        impl->colormap->setColormap(settings.value("cmap").toInt());
        impl->Wsmoothing->setSmoothing({ settings.value("smoothing0", 0.).toInt(), settings.value("smoothing1", 5).toInt(), settings.value("smoothing2", 0.).toDouble(), settings.value("smoothing3", 0.).toDouble() });
        impl->projection->setProjection(settings.value("proj", 0).toInt());
    }
    settings.endGroup();
    impl->Wsmoothing->updateSmothingParamWidgets();
    emit imgSettingsChanged(impl->updateSettings());
    
}
void ImageSettingsWidget::resetSettings() {

    ROIVert::imgsettings defaultSettings;
    impl->dispsettings->setContrast(false, defaultSettings.Contrast);
    impl->dispsettings->setContrast(true, defaultSettings.Contrast);

    setContrast(defaultSettings.Contrast);

    impl->colormap->setColormap(defaultSettings.cmap);
    impl->Wsmoothing->setSmoothing(defaultSettings.Smoothing);
    impl->projection->setProjection(0);
    
    impl->Wsmoothing->updateSmothingParamWidgets();
    emit imgSettingsChanged(defaultSettings);
}
bool ImageSettingsWidget::isProjectionActive() const {
    return impl->projection->getProjection() > 0;
}