#ifndef FUZZYDROPLETS_GUI_PLOT_MARKER2D_H
#define FUZZYDROPLETS_GUI_PLOT_MARKER2D_H

#include "marker.h"
#include "continuousaxis.h"

namespace Plot
{

class Marker2D : public Primitive
{
public:

    Marker2D(Marker::Shape shape, ContinuousAxis * xAxis, ContinuousAxis * yAxis, double x, double y)
        : Primitive(), m_x(x), m_y(y), m_xAxis(xAxis), m_yAxis(yAxis)
    {
        static const double size = 12;
        switch (shape) {
            case Marker::Square : m_marker = new SquareMarker(size); m_pen = Qt::NoPen; m_brush = Qt::black; break;
            case Marker::Circle : m_marker = new CircleMarker(size); m_pen = Qt::NoPen; m_brush = Qt::black; break;
            case Marker::Diamond : m_marker = new DiamondMarker(size); m_pen = Qt::NoPen; m_brush = Qt::black; break;
            case Marker::Cross : m_marker = new CrossMarker(size); m_pen = QPen(Qt::black, 1); m_brush = Qt::NoBrush; break;
            case Marker::X : m_marker = new XMarker(size); m_pen = QPen(Qt::black, 1); m_brush = Qt::NoBrush; break;
            case Marker::Star : m_marker = new StarMarker(size); m_pen = Qt::NoPen; m_pen.setJoinStyle(Qt::MiterJoin); m_brush = Qt::black; break;
            case Marker::LeftTriangle : m_marker = new LeftTriangleMarker(size); m_pen = Qt::NoPen; m_brush = Qt::black; break;
            case Marker::RightTriangle : m_marker = new RightTriangleMarker(size); m_pen = Qt::NoPen; m_brush = Qt::black; break;
            case Marker::UpTriangle : m_marker = new UpTriangleMarker(size); m_pen = Qt::NoPen; m_brush = Qt::black; break;
            case Marker::DownTriangle : m_marker = new DownTriangleMarker(size); m_pen = Qt::NoPen; m_brush = Qt::black; break;
        };
    }

    ~Marker2D()
    {
        delete m_marker;
    }

    void setValue(double x, double y) {m_x = x; m_y = y;}
    void setX(double x) {m_x = x;}
    void setY(double y) {m_y = y;}
    double x() const {return m_x;}
    double y() const {return m_y;}
    void setSize(double size) {m_marker->setSize(size);}
    double size() const {return m_marker->size();}
    ContinuousAxis * xAxis() const {return m_xAxis;}
    ContinuousAxis * yAxis() const {return m_yAxis;}
    void setPen(const QPen & pen) {m_pen = pen;}
    void setBrush(const QBrush & brush) {m_brush = brush;}
    const QPen & pen() const {return m_pen;}
    const QBrush & brush() const {return m_brush;}

    void render(QPainter & painter)
    {
        if (this->isVisible()) {
            apply(painter, pen(), brush(), QPalette::Text, QPalette::Text);
            m_marker->paint(painter, xAxis()->pixel(x()), yAxis()->pixel(y()));
        }
    }

protected:

    void drawTarget(QPainter & painter)
    {
        painter.setPen(QPen(Qt::red, 2));
        painter.drawLine(QPointF(xAxis()->pixel(x()), yAxis()->pixel(y())), QPointF(xAxis()->pixel(x()), yAxis()->pixel(y())));
    }

    void drawRect(QPainter & painter)
    {
        painter.setPen(QPen(Qt::red, 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(QRectF(QPointF(xAxis()->pixel(x()) - size() / 2, yAxis()->pixel(y()) - size() / 2), QPointF(xAxis()->pixel(x()) + size() / 2, yAxis()->pixel(y()) + size() / 2)));
    }

    double m_x {0};
    double m_y {0};
    ContinuousAxis * m_xAxis;
    ContinuousAxis * m_yAxis;
    QPen m_pen {Qt::NoPen};
    QBrush m_brush {Qt::black};
    Marker * m_marker {nullptr};
};

} // namespace Plot

#endif // FUZZYDROPLETS_GUI_PLOT_MARKER2D_H
