#pragma once

class QPainter;
class QFont;
class QString;
namespace TraceChart {
    struct AxisStyle;
};

#include <tuple>
#include <memory>

class Axis
{
public:
    Axis();
    virtual ~Axis();
    virtual void paint(QPainter& painter) = 0;

    void setExtents(const double& min, const double& max);
    std::tuple<double, double> getExtents() const noexcept;
    std::tuple<double, double> getLimits() const;

    void setLabel(const QString& Label);
    virtual QString getLabel() const noexcept;

    virtual void setLength(const int& length) = 0;  // Length corresponds to the direction of the axle ** note that this doesn't affect layout!
    virtual int getLength() const noexcept = 0;

    virtual std::tuple<double, double> getMargins() const = 0;   // The length corresponds to the length of the axle, margins include any text that overhangs
    virtual int getThickness() const noexcept = 0;                        // Thickness is in the perpendecular direction to the axis. 

    void setZero(const int& xzero, const int& yzero) noexcept;
    void setTickFont(const QFont& f);
    void setLabelFont(const QFont& f);

    // this is the spacing past...
    void setSpacings(const int& label, const int& ticklabel, const int& tickmark) noexcept;
    void setTickLength(const int& ticklength) noexcept;

    void setMaxNTicks(const unsigned int& n);


    void setStyle(TraceChart::AxisStyle);
    TraceChart::AxisStyle getStyle();

protected:
    // this updates the tick values, the ticklabelthickness, and the margins. It needs to be called if the font changes or the labels change.
    virtual void updateLayout();
    struct pimpl;
    std::unique_ptr<pimpl> impl = std::make_unique<pimpl>();
};

class HAxis : public Axis {
public:
    void paint(QPainter& painter) override;
    void setLength(const int& length) noexcept override;
    int getLength() const noexcept override;
    std::tuple<double, double> getMargins() const override;
    int getThickness() const noexcept override;

protected:
    void updateLayout() override;
};

class VAxis : public Axis {
public:
    void paint(QPainter& painter) override;
    void setLength(const int& length) noexcept override;
    int getLength() const noexcept override;
    std::tuple<double, double> getMargins() const override;
    int getThickness() const noexcept override;

protected:
    void updateLayout() override;

};