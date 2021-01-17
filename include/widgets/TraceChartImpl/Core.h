#pragma once

#include <QColor>
#include <QFont>
#include <cmath>


namespace TraceChart {
    enum class NORM {
        NONE,
        ZEROTOONE,
        L1NORM,
        L2NORM,
        ZSCORE,
        MEDIQR
    };
    struct AxisStyle {
        QColor Color = Qt::black;
        QFont LabelFont;
        QFont TickLabelFont;
        double LineWidth = 1.;
        bool Show = true;
        bool ShowX = true;
        bool ShowY = true;
    };
    struct LineStyle {
        std::vector<QColor> Colors = { Qt::black };

        bool Fill = false;
        bool FillGradientToLineColor = false;

        // not recommended to play with these...
        float FillGradientExponent = 4.;
        float FillGradientDivisor = 3.;
        std::vector<QColor> FillColors = { Qt::black };

        double Width = 3;

    };
    struct ChartStyle
    {
        ChartStyle() {
            // just some default font hier
            QFont f;
            f.setPointSize(12);
            Axis.TickLabelFont = f;

            f.setBold(true);
            Axis.LabelFont = f;

            f.setPointSize(14);
            Title.Font = f;
        }
        AxisStyle Axis;

        struct BackgroundStyle {
            QColor Color = Qt::white;
        } Background;

        struct TitleStyle {
            QColor Color = Qt::black;
            QFont Font;
        } Title;

        LineStyle Line;
    };

    static const std::vector<QColor> paletteColors() {
        const std::vector<QColor> sclr = { QColor("#FFBE00"),QColor("#0055C8"),QColor("#6E6E00"),QColor("#82C8E6"),QColor("#A0006E"),
                                      QColor("#FF5A00"),QColor("#82DC73"),QColor("#FF82B4"),QColor("#D2D200"),QColor("#D282BE"),
                                      QColor("#DC9600"),QColor("#6E491E"),QColor("#00643C"),QColor("#82C8E6"),QColor("#640082"),
                                      QColor("#A81032"),QColor("#96C047"),QColor("#E170A7"),QColor("#147A88") };
        return sclr;
    };


    namespace {
        double niceNum(const double& range, const bool round) noexcept {

            float exponent = std::floor(std::log10(range));
            float fraction = range / std::pow(10.f, exponent);
            float niceFraction = 0.; /** nice, rounded fraction */

            if (round)
            {
                if (fraction < 1.5)
                    niceFraction = 1;
                else if (fraction < 3)
                    niceFraction = 2;
                else if (fraction < 7)
                    niceFraction = 5;
                else
                    niceFraction = 10;
            }
            else
            {
                if (fraction <= 1)
                    niceFraction = 1;
                else if (fraction <= 2)
                    niceFraction = 2;
                else if (fraction <= 5)
                    niceFraction = 5;
                else
                    niceFraction = 10;
            }

            return niceFraction * pow(10, exponent);
        }
    }

    namespace tickpicker {
        static std::vector<double> getNiceTicksLimits(double minval, const double& maxval, const unsigned int& maxticks) {
            // https://stackoverflow.com/questions/8506881/nice-label-algorithm-for-charts-with-minimum-ticks
            // might still be some bugs here around floating point precision, hard to fgigure out how to not make a mess out of this!

            const auto range = niceNum(maxval - minval, false);
            const auto spacing = niceNum(range / (maxticks - 1), true);

            //spacing = std::round(spacing / spacing_round_fac) * spacing_round_fac;

            auto nicemin = std::floor(minval / spacing) * spacing;
            const auto nicemax = ceil(maxval / spacing) * spacing;

            // rfac is an attempt at dealing with floating point precision issues:
            //   when floor jumps down too much, increment nicemin by one spacing unit
            //      similar for maxval..
            const double rfac = std::pow(10, std::floor(log10(spacing)) - 5);

            if (std::abs(minval - (nicemin + spacing)) < rfac) {
                nicemin += spacing;
            }

            std::vector<double> ticks = { nicemin };

            while ((maxval - ticks.back()) > rfac) {
                ticks.push_back(ticks.back() + spacing);
            }
            return ticks;
        };
    }
};