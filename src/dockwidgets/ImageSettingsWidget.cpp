#include "dockwidgets/ImageSettingsWidget.h"

#include <QBoxLayout>
#include <QLabel>

#include "widgets/ContrastWidget.h"
#include "widgets/ProjectionPickWidget.h"
#include "widgets/ColormapPickWidget.h"
#include "widgets/SmoothingPickWidget.h"

static void addVSep(QVBoxLayout *lay) {
    QFrame* line = new QFrame;
    line->setFrameStyle(QFrame::HLine);
    lay->addWidget(line);

}

struct ImageSettingsWidget::pimpl {
    QWidget* contents;

    ContrastWidget* contrast = new ContrastWidget;
    ProjectionPickWidget* projection = new ProjectionPickWidget;
    ColormapPickWidget* colormap = new ColormapPickWidget;;
    SmoothingPickWidget* Wsmoothing = new SmoothingPickWidget;

    void init() {
        contrast->setMaximumHeight(300);
        contrast->setMaximumWidth(300);
        projection->setMaximumWidth(280);
        colormap->setMaximumWidth(300);
        Wsmoothing->setMaximumWidth(300);
    }

    void layout(QVBoxLayout* topLay) {
        topLay->setAlignment(Qt::AlignTop);

        topLay->addWidget(new QLabel(tr("Contrast:")));
        topLay->addWidget(contrast);
        addVSep(topLay);
        topLay->addWidget(new QLabel(tr("Projection:")));
        topLay->addWidget(projection);
        addVSep(topLay);
        topLay->addWidget(new QLabel(tr("Colormap:")));
        topLay->addWidget(colormap);
        addVSep(topLay);
        topLay->addWidget(new QLabel(tr("Smoothing:")));
        topLay->addWidget(Wsmoothing);
        topLay->addStretch(1);
    }

    ROIVert::imgsettings updateSettings() {
        // this is a centralized location for updating settings and emitting a signal with the whole payload:
        ROIVert::imgsettings payload;
        payload.Contrast = contrast->getContrast();
        payload.projectionType = projection->getProjection();
        payload.cmap = colormap->getColormap();
        payload.Smoothing = Wsmoothing->getSmoothing();

        return payload;
    }
};

ImageSettingsWidget::ImageSettingsWidget(QWidget* parent) : QDockWidget(parent) {
    impl->contents = new QWidget;
    this->setWidget(impl->contents);
    QVBoxLayout* lay = new QVBoxLayout;
    impl->contents->setLayout(lay);

    impl->init();
    impl->layout(lay);
    
    auto lam = [=] { 
        auto newsettings = impl->updateSettings(); 
        emit imgSettingsChanged(newsettings); 
    };

    connect(impl->contrast, &ContrastWidget::contrastChanged, this, lam);
    connect(impl->projection, &ProjectionPickWidget::projectionChanged, this, lam);
    connect(impl->colormap, &ColormapPickWidget::colormapChanged, this, lam);
    connect(impl->Wsmoothing, &SmoothingPickWidget::smoothingChanged, this, lam);
}
ImageSettingsWidget::~ImageSettingsWidget() = default;


void ImageSettingsWidget::setHistogram(std::vector<float> &data){
    // todo: consider whether this should come in as a QVector to avoid the conversion (or vice versa)
    impl->contrast->setHistogram(QVector<float>::fromStdVector(data));
}
void ImageSettingsWidget::setContrast(ROIVert::contrast c){
    impl->contrast->setContrast(c);
}

void ImageSettingsWidget::setContentsEnabled(bool onoff) {
    impl->contents->setEnabled(onoff);
}