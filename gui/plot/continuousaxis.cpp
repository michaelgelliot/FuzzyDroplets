#include "continuousaxis.h"
#include <QFontMetrics>
#include <QPainter>

namespace Plot
{

ContinuousAxis::ContinuousAxis()
    : Axis()
{
    setVisibility(Line | Label | MajorTick | MajorTickLabel | MediumTick | MinorTick, true);
    updateNumberSize();
}

double ContinuousAxis::firstMajorTick() const
{
    double result = m_majorTickUnit * (int)(m_min / m_majorTickUnit);
    return (result < m_min) ? result + m_majorTickUnit : result;
}

double ContinuousAxis::lastMajorTick() const
{
    double result = m_majorTickUnit * (int)(m_max / m_majorTickUnit);
    return (result + m_majorTickUnit <= m_max) ? result + m_majorTickUnit : result;
}

double ContinuousAxis::firstMediumTick() const
{
    double result = m_mediumTickUnit * (int)(m_min / m_mediumTickUnit);
    return (result < m_min) ? result + m_mediumTickUnit : result;
}

double ContinuousAxis::lastMediumTick() const
{
    double result = m_mediumTickUnit * (int)(m_max / m_mediumTickUnit);
    return (result + m_mediumTickUnit <= m_max) ? result + m_mediumTickUnit : result;
}

double ContinuousAxis::firstMinorTick() const
{
    double result = m_minorTickUnit * (int)(m_min / m_minorTickUnit);
    return (result < m_min) ? result + m_minorTickUnit : result;
}

double ContinuousAxis::lastMinorTick() const
{
    double result = m_minorTickUnit * (int)(m_max / m_minorTickUnit);
    return (result + m_minorTickUnit <= m_max) ? result + m_minorTickUnit : result;
}

int ContinuousAxis::numMajorTicks() const
{
    return round((lastMajorTick() - firstMajorTick()) / m_majorTickUnit + 1);
}

int ContinuousAxis::numMediumTicks() const
{
    return round((lastMediumTick() - firstMediumTick()) / m_mediumTickUnit + 1);
}

int ContinuousAxis::numMinorTicks() const
{
    return round((lastMinorTick() - firstMinorTick()) / m_minorTickUnit + 1);
}

void ContinuousAxis::updateNumberSize()
{
    if (isVisible(MajorTickLabel)) {
        QFontMetrics fm(majorTickLabelFont());
        QSizeF size(0,0);
        double firstTick = firstMajorTick();
        int numTicks = numMajorTicks();
        for (int i = 0; i < numTicks; ++i) {
            QString str = m_locale.toString(firstTick + i * m_majorTickUnit, m_localeFormat, m_localePrecision);
            auto br = fm.tightBoundingRect(str);
            size.setWidth(std::max(size.width(), (double)br.width() + 1));
            size.setHeight(std::max(size.height(), (double)br.height() + 1));
        }
        setMajorTickLabelSize(size);
    }
}

QPointF ContinuousAxis::firstTickPos(Axis::Component tickType, double firstTick, double tickLength)
{
    if (m_inverted) {
        return (orientation() == Qt::Vertical) ? QPointF(alignment(tickType) == AlignLeft ? -(isVisible(tickType) ? tickLength : 0) : 0, ((firstTick - m_max) * pixelLength())/(m_max - m_min)) : QPointF(((firstTick - m_max) * pixelLength())/(m_min - m_max), alignment(tickType) == AlignLeft ? (isVisible(tickType) ? tickLength : 0) : 0);
    } else {
        return (orientation() == Qt::Vertical) ? QPointF(alignment(tickType) == AlignLeft ? -(isVisible(tickType) ? tickLength : 0) : 0, ((firstTick - m_min) * pixelLength())/(m_min - m_max)) : QPointF(((firstTick - m_min) * pixelLength())/(m_max - m_min), alignment(tickType) == AlignLeft ? (isVisible(tickType) ? tickLength : 0) : 0);
    }
}

QPointF ContinuousAxis::tickPixelUnits(double tickUnit)
{
    if (m_inverted) {
        return (orientation() == Qt::Vertical) ? QPointF(0, tickUnit * pixelLength() / (m_max - m_min)) : QPointF(-tickUnit * pixelLength() / (m_max - m_min), 0);
    } else {
        return (orientation() == Qt::Vertical) ? QPointF(0, -tickUnit * pixelLength() / (m_max - m_min)) : QPointF(tickUnit * pixelLength() / (m_max - m_min), 0);
    }
}

QPointF ContinuousAxis::tickStep(Axis::Component tickType, double tickLength)
{
    return (orientation() == Qt::Vertical) ? QPointF((isVisible(tickType) ? tickLength : 0), 0) : QPointF(0, -(isVisible(tickType) ? tickLength : 0));
}

void ContinuousAxis::render(QPainter & paint, bool highQualityOutput)
{
    paint.save();

    double scale = pixelLength() / valueLength();

    // major ticks

    if (isVisible(MajorTick) && m_majorTickUnit * scale > 2 * pen(MajorTick).widthF()) {
        apply(paint, pen(MajorTick), QPalette::Text);
        auto tickPos = firstTickPos(MajorTick, firstMajorTick(), majorTickLength());
        auto tickPixels = tickPixelUnits(m_majorTickUnit);
        auto tickLine = tickStep(MajorTick, majorTickLength());
        int num = numMajorTicks();
        for (int i = 0; i < num; ++i) {
            paint.drawLine(tickPos, tickPos + tickLine);
            tickPos += tickPixels;
        }
    }

    // numbers

    double m_charWidth = 6;
    if (isVisible(MajorTickLabel)) {
        paint.setFont(this->majorTickLabelFont());
        apply(paint, pen(MajorTickLabel), QPalette::Text);
        double firstTick = firstMajorTick();
        auto tickPos = firstTickPos(MajorTick, firstMajorTick(), majorTickLength());
        auto tickPixels = tickPixelUnits(m_majorTickUnit);
        auto tickLine = tickStep(MajorTick, majorTickLength());
        int num = numMajorTicks();
        if (orientation() == Qt::Horizontal) {
            tickPos.setX(tickPos.x() - majorTickLabelSize().width() / 2);
            tickPos.setY((alignment(Axis::MajorTickLabel) == Axis::AlignLeft) ? tickPos.y() + m_charWidth: tickPos.y() + tickLine.y() - majorTickLabelSize().height() - m_charWidth);
            int skip = int(majorTickLabelSize().width() / tickPixels.x()) + 1;
            if (skip > 0) {
                for (int i = 0; i < num; ++i) {
                    if (i % skip == 0)
                        paint.drawText(QRectF(tickPos.x(), tickPos.y(), majorTickLabelSize().width(), majorTickLabelSize().height()), Qt::AlignCenter, m_locale.toString(firstTick + i * m_majorTickUnit, m_localeFormat, m_localePrecision));
                    tickPos += tickPixels;
                }
            }
        } else {
            tickPos.setY(tickPos.y() - majorTickLabelSize().height() / 2);
            tickPos.setX((alignment(Axis::MajorTickLabel) == Axis::AlignLeft) ? tickPos.x() - majorTickLabelSize().width() - m_charWidth: tickPos.x() + tickLine.x() + m_charWidth);
            int skip = int(-majorTickLabelSize().height() / tickPixels.y()) + 1;
            if (skip > 0) {
                for (int i = 0; i < num; ++i) {
                    if (i % skip == 0)
                        paint.drawText(QRectF(tickPos.x(), tickPos.y(), majorTickLabelSize().width(), majorTickLabelSize().height()), alignment(MajorTickLabel) == Axis::AlignLeft ? Qt::AlignVCenter | Qt::AlignRight : Qt::AlignVCenter | Qt::AlignLeft, m_locale.toString(firstTick + i * m_majorTickUnit, m_localeFormat, m_localePrecision));
                    tickPos += tickPixels;
                }
            }
        }
    }

    // medium ticks

    if (isVisible(MediumTick) && m_mediumTickUnit * scale > 2 * pen(MediumTick).widthF()) {
        paint.setFont(this->mediumTickLabelFont());
        apply(paint, pen(MediumTick), QPalette::Text);
        auto tickPos = firstTickPos(MediumTick, firstMediumTick(), mediumTickLength());
        auto tickPixels = tickPixelUnits(m_mediumTickUnit);
        auto tickLine = tickStep(MediumTick, mediumTickLength());
        int num = numMediumTicks();
        for (int i = 0; i < num; ++i) {
            paint.drawLine(tickPos, tickPos + tickLine);
            tickPos += tickPixels;
        }
    }

    // minor ticks

    if (isVisible(MinorTick) && m_minorTickUnit * scale > 2 * pen(MinorTick).widthF()) {
        paint.setFont(this->minorTickLabelFont());
        apply(paint, pen(MinorTick), QPalette::Text);
        auto tickPos = firstTickPos(MinorTick, firstMinorTick(), minorTickLength());
        auto tickPixels = tickPixelUnits(m_minorTickUnit);
        auto tickLine = tickStep(MinorTick, minorTickLength());
        int num = numMinorTicks();
        for (int i = 0; i < num; ++i) {
            paint.drawLine(tickPos, tickPos + tickLine);
            tickPos += tickPixels;
        }
    }

    paint.restore();

    Axis::render(paint, highQualityOutput);
}

void ContinuousAxis::setupFromDataRange(double low, double high)
{
    auto z = log10(high - low);
    m_majorTickUnit = pow(10, z < 0 ? (int)(z - 1) : (int)z);

    auto q = m_majorTickUnit / 100;
    std::array<double, 8> blipMajor =  {100.0*q, 50.0*q, 25.0*q, 20.0*q, 10.0*q};
    std::array<double, 8> blipMedium = {50.0*q, 25.0*q, 5.0*q, 10.0*q, 5.0*q};
    std::array<double, 8> blipMinor =  {10.0*q,  5.0*q, 1.0*q,  2.0*q,  1.0*q};
    m_mediumTickUnit = blipMedium[0];
    m_minorTickUnit = blipMinor[0];
    int i = 0;
    while (i < blipMajor.size() && (high - low)/m_majorTickUnit < 5) {
        ++i;
        m_majorTickUnit = blipMajor[i];
        m_mediumTickUnit = blipMedium[i];
        m_minorTickUnit = blipMinor[i];
    }
    m_min = m_majorTickUnit * (int)(low/m_majorTickUnit) ;
    m_max = m_min + (m_majorTickUnit * (1 + (int)((high - low)/m_majorTickUnit) ));
    if (m_max < high) m_max += m_majorTickUnit;

    z = log10(m_majorTickUnit);
    if (z < 0) {
        setLocaleFormat('f', std::ceil(-z));
    } else {
        setLocaleFormat('f', 0);
    }

    m_absoluteMax = m_max;
    m_absoluteMin = m_min;
    updateNumberSize();
}

} // namespace Plot
