#ifndef FUZZYDROPLETS_GUI_PLOT_CONTINUOUSAXIS_H
#define FUZZYDROPLETS_GUI_PLOT_CONTINUOUSAXIS_H

#include "axis.h"
#include <QLocale>

namespace Plot
{

class ContinuousAxis : public Axis
{
public:

    ContinuousAxis();

    double valueLength() const {return m_max - m_min;}
    double absoluteValueLength() const {return m_absoluteMax - m_absoluteMin;}

    bool isInverted() const {return m_inverted;}
    void setInverted(bool inverted) {m_inverted = inverted;}

    double minimum() const {return m_min;}
    double maximum() const {return m_max;}
    void setMinimum(double minimum) {m_min = std::clamp(minimum, m_absoluteMin, m_absoluteMax);}
    void setMaximum(double maximum) {m_max = std::clamp(maximum, m_absoluteMin, m_absoluteMax);}
    void setRange(double minimum, double maximum) {m_min = std::clamp(minimum, m_absoluteMin, m_absoluteMax); m_max = std::clamp(maximum, m_absoluteMin, m_absoluteMax);}

    double absoluteMinimum() const {return m_absoluteMin;}
    double absoluteMaximum() const {return m_absoluteMax;}
    void setAbsoluteMinimum(double minimum) {m_absoluteMin = minimum; m_min = std::max(m_absoluteMin, m_absoluteMin);}
    void setAbsoluteMaximum(double maximum) {m_absoluteMax = maximum; m_max = std::min(m_absoluteMax, m_absoluteMax);}
    void setAbsoluteRange(double minimum, double maximum) {m_absoluteMin = minimum; m_absoluteMax = maximum; m_min = std::clamp(m_min, m_absoluteMin, m_absoluteMax); m_max = std::clamp(m_max, m_absoluteMin, m_absoluteMax);}

    void setupFromDataRange(double minimum, double maximum);

    double majorTickUnit() const {return m_majorTickUnit;}
    void setMajorTickUnit(double unit) {m_majorTickUnit = unit;}
    inline double firstMajorTick() const;
    inline double lastMajorTick() const;
    inline int numMajorTicks() const;

    double mediumTickUnit() const {return m_mediumTickUnit;}
    void setMediumTickUnit(double unit) {m_mediumTickUnit = unit;}
    inline double firstMediumTick() const;
    inline double lastMediumTick() const;
    inline int numMediumTicks() const;

    double minorTickUnit() const {return m_minorTickUnit;}
    void setMinorTickUnit(double unit) {m_minorTickUnit = unit;}
    inline double firstMinorTick() const;
    inline double lastMinorTick() const;
    inline int numMinorTicks() const;

    void setLocale(QLocale locale) {m_locale = locale; updateNumberSize();}
    void setLocaleFormat(char format, int precision) {m_localeFormat = format; m_localePrecision = precision; updateNumberSize();}
    const QLocale & locale() const {return m_locale;}
    std::tuple<char, int> localeFormat() const {return {m_localeFormat, m_localePrecision};}

    double value(double pixel) const {return ((m_inverted && orientation() == Qt::Horizontal) || (!m_inverted && orientation() == Qt::Vertical)) ? m_max - (pixel / pixelLength()) * (m_max - m_min) : (pixel / pixelLength()) * (m_max - m_min) + m_min;}
    double pixel(double value) const {return ((m_inverted && orientation() == Qt::Horizontal) || (!m_inverted && orientation() == Qt::Vertical)) ? ((pixelLength() * (m_max - value))/(m_max - m_min)) : -((pixelLength() * (m_min - value))/(m_max - m_min));}

    virtual void render(QPainter & painter, bool highQualityOutput = false) override;

protected:

    void updateNumberSize();

private:

    QPointF firstTickPos(Axis::Component tickType, double firstTick, double tickLength);
    QPointF tickPixelUnits(double tickUnit);
    QPointF tickStep(Axis::Component tickType, double tickLength);

    double m_min {0};
    double m_max {1};
    double m_absoluteMin {-std::numeric_limits<double>::infinity()};
    double m_absoluteMax {std::numeric_limits<double>::infinity()};
    double m_majorTickUnit {0.1};
    double m_mediumTickUnit {0.05};
    double m_minorTickUnit {0.01};
    bool m_inverted {false};

    QLocale m_locale;
    char m_localeFormat {'f'};
    int m_localePrecision {1};
};

} // namespace Plot

#endif // FUZZYDROPLETS_GUI_PLOT_CONTINUOUSAXIS_H
