#include "polygon.h"
#include <QPolygon>
#include <QPainter>

namespace Plot {

Polygon::Polygon(ContinuousAxis * xAxis, ContinuousAxis * yAxis)
    : Primitive(),
    m_xAxis(xAxis),
    m_yAxis(yAxis)
{
}

void Polygon::render(QPainter & painter)
{
    if (!isVisible()) return;
    painter.save();
    apply(painter, m_pen, m_brush, QPalette::Text, QPalette::Text);
    painter.setRenderHint(QPainter::Antialiasing, antialiased());
    QPolygonF poly;
    for (int i = 0; i < m_polygon.size(); ++i) {
        poly.push_back(QPointF(m_xAxis->pixel(m_polygon.at(i).x()), m_yAxis->pixel(m_polygon.at(i).y())));
    }
    painter.drawPolygon(poly);
    painter.restore();
}

} // namespace Plot
