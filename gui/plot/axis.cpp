#include "axis.h"
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEngine>

namespace Plot
{

Axis::Axis()
    : Primitive()
{
    m_componentVisibility.fill(false);
    setVisibility(Line | Label, true);
    m_alignment.fill(AlignLeft);
}

void Axis::updateLabelBoundingRect()
{
    if (m_label.isEmpty()) {
        m_labelBoundingRect = QRectF(0,0,0,0);
    } else {
        QFontMetrics fm(m_labelFont);
        m_labelBoundingRect = fm.boundingRect(m_label);
    }
}

void Axis::updateLabelPixmap(double devicePixelRatio)
{
    m_devicePixelRatio = devicePixelRatio;
    if (m_orientation == Qt::Vertical) {
        m_labelPixmap = QPixmap(m_labelBoundingRect.width() * devicePixelRatio + 2, m_labelBoundingRect.height() * devicePixelRatio + 2);
        m_labelPixmap.setDevicePixelRatio(devicePixelRatio);
        m_labelPixmap.fill(Qt::transparent);
        QPainter paint(&m_labelPixmap);
        apply(paint, pen(Label), QPalette::Text);
        paint.setFont(m_labelFont);
        paint.drawText(QRectF(0, 0, (double)m_labelPixmap.width() / devicePixelRatio, (double)m_labelPixmap.height() / devicePixelRatio), Qt::AlignCenter, m_label);
    }
}

double Axis::offset(Alignment a) const
{
    double tickSize = std::max((isVisible(MajorTick) && alignment(MajorTick) == a) * m_majorTickLength,
                      std::max((isVisible(MediumTick) && alignment(MediumTick) == a) * m_mediumTickLength,
                               (isVisible(MinorTick) && alignment(MinorTick) == a) * m_minorTickLength));

    double tickLabelSpace = (orientation() == Qt::Horizontal) ?
                            std::max((isVisible(MajorTickLabel) && alignment(MajorTickLabel) == a) * m_majorTickLabelSize.height(),
                            std::max((isVisible(MediumTickLabel) && alignment(MediumTickLabel) == a) * m_mediumTickLabelSize.height(),
                                     (isVisible(MinorTickLabel) && alignment(MinorTickLabel) == a) * m_minorTickLabelSize.height()))
                          : std::max((isVisible(MajorTickLabel) && alignment(MajorTickLabel) == a) * m_majorTickLabelSize.width(),
                            std::max((isVisible(MediumTickLabel) && alignment(MediumTickLabel) == a) * m_mediumTickLabelSize.width(),
                                     (isVisible(MinorTickLabel) && alignment(MinorTickLabel) == a) * m_minorTickLabelSize.width()));

    double labelSize = (isVisible(Label) && alignment(MinorTickLabel) == a && !m_label.isEmpty()) * (m_padding + m_labelBoundingRect.height());

    return labelSize + tickSize + tickLabelSpace;
}

void Axis::render(QPainter & paint)
{
    paint.save();

    if (isVisible(Line)) {
        apply(paint, pen(Line), QPalette::Text);
        paint.drawLine(QPointF(0, 0), (m_orientation == Qt::Horizontal) ? QPointF(m_pixelLength, 0) : QPointF(0, -m_pixelLength));
    }

    if (isVisible(Label) && !m_label.isEmpty()) {
        apply(paint, pen(Label), QPalette::Text);
        paint.setFont(m_labelFont);
        QRectF textRect;
        if ((alignment(Label) == AlignLeft && m_orientation == Qt::Horizontal) || (alignment(Label) == AlignRight && m_orientation == Qt::Vertical))
            textRect = {0, (alignment(Label) == AlignLeft ? leftOffset() : rightOffset()) - m_labelBoundingRect.height(), m_pixelLength, m_labelBoundingRect.height()};
        else
            textRect = {0, -(alignment(Label) == AlignLeft ? leftOffset() : rightOffset()), m_pixelLength, m_labelBoundingRect.height()};
        auto paintType = paint.paintEngine()->type();
        if (m_orientation == Qt::Horizontal || paintType == QPaintEngine::SVG || paintType == QPaintEngine::Pdf) {
            if (m_orientation == Qt::Vertical)
                paint.rotate(-90);
            paint.drawText(textRect, Qt::AlignCenter, m_label);
            if (m_orientation == Qt::Vertical)
                paint.rotate(90);
        } else {
            if (paint.device()->devicePixelRatio() != m_devicePixelRatio)
                updateLabelPixmap(paint.device()->devicePixelRatio());
            paint.rotate(-90);
            paint.drawPixmap(textRect.center().x() - m_labelBoundingRect.width() / 2, textRect.center().y() - m_labelBoundingRect.height() / 2, m_labelPixmap);
            paint.rotate(90);
        }
    }

    paint.restore();
}

} // namespace Plot
