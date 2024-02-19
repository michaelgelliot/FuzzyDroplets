#ifndef FUZZYDROPLETS_GUI_PLOT_POLYGON_H
#define FUZZYDROPLETS_GUI_PLOT_POLYGON_H

#include "primitive.h"
#include "continuousaxis.h"

namespace Plot {

class Polygon : public Primitive
{
public:

    Polygon(ContinuousAxis * xAxis, ContinuousAxis * yAxis);

    void setPolygon(const QPolygonF & polygon) {m_polygon = polygon;}
    void setPen(const QPen & pen) {m_pen = pen;}
    void setBrush(const QBrush & brush) {m_brush = brush;}

    void render(QPainter & painter);

private:

    ContinuousAxis * m_xAxis {nullptr};
    ContinuousAxis * m_yAxis {nullptr};
    QPolygonF m_polygon;
    QPen m_pen {QPen(Qt::black, 1)};
    QBrush m_brush {Qt::transparent};
};

} // namespace Plot

#endif // FUZZYDROPLETS_GUI_PLOT_POLYGON_H
