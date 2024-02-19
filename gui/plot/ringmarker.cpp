#include "ringmarker.h"
#include <QPainter>

namespace Plot
{

RingMarker::RingMarker(ContinuousAxis * xAxis, ContinuousAxis * yAxis, double x, double y)
    : Primitive(),
    m_x(x),
    m_y(y),
    m_xAxis(xAxis),
    m_yAxis(yAxis)
{
}

void RingMarker::render(QPainter & painter)
{
    if (this->isVisible()) {
        painter.save();
        painter.setPen(Qt::NoPen);

        painter.setBrush(Qt::NoBrush);
        double outerThickness = m_pen.widthF() + 4;

        QPen pen(Qt::white, outerThickness);
        apply(painter, pen, QPalette::Base);
        painter.drawEllipse(QRectF(xAxis()->pixel(m_x) - m_size/2 + outerThickness/2, yAxis()->pixel(m_y) - m_size/2 + outerThickness/2, m_size - outerThickness, m_size - outerThickness));

        pen = QPen(Qt::black, outerThickness - 2);
        apply(painter, pen, QPalette::Text);
        painter.drawEllipse(QRectF(xAxis()->pixel(m_x) - m_size/2 + outerThickness/2, yAxis()->pixel(m_y) - m_size/2 + outerThickness/2, m_size - outerThickness, m_size - outerThickness));

        apply(painter, m_pen, QPalette::Text);
        painter.drawEllipse(QRectF(xAxis()->pixel(m_x) - m_size/2 + outerThickness/2, yAxis()->pixel(m_y) - m_size/2 + outerThickness/2, m_size - outerThickness, m_size - outerThickness));

        painter.restore();
    }
}

}
