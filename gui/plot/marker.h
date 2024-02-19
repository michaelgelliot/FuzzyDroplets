#ifndef FUZZYDROPLETS_GUI_PLOT_MARKER_H
#define FUZZYDROPLETS_GUI_PLOT_MARKER_H

#include <QPainter>

namespace Plot
{

class Marker
{
public:

    enum Shape
    {
        Square,
        Circle,
        Diamond,
        Cross,
        X,
        Star,
        LeftTriangle,
        RightTriangle,
        UpTriangle,
        DownTriangle
    };

    Marker(double size) : m_size(size) {}
    virtual ~Marker() {}
    double size() const {return m_size;}
    virtual void setSize(double size) {m_size = size;}
    virtual void paint(QPainter & painter, double x, double y) = 0;

protected:

    double m_size;
};

class SquareMarker : public Marker
{
public:

    SquareMarker(double size) : Marker(size) {precompute(size);}

    void paint(QPainter & painter, double x, double y)
    {
        painter.drawRect(m_rect.translated(x,y));
    }

    void setSize(double size)
    {
        Marker::setSize(size);
        precompute(size);
    }

private:

    void precompute(double size)
    {
        m_rect = QRectF(-m_size/2, -m_size/2, m_size, m_size);
    }

    QRectF m_rect;
};

class CircleMarker : public Marker
{
public:

    CircleMarker(double size) : Marker(size) {precompute(size);}

    void paint(QPainter & painter, double x, double y)
    {
        painter.drawEllipse(m_rect.translated(x,y));
    }

    void setSize(double size)
    {
        Marker::setSize(size);
        precompute(size);
    }

private:

    void precompute(double size)
    {
        m_rect = QRectF(-m_size/2, -m_size/2, m_size, m_size);
    }

    QRectF m_rect;
};

class DiamondMarker : public Marker
{
public:

    DiamondMarker(double size) : Marker(size) {precompute(size);}

    void paint(QPainter & painter, double x, double y)
    {
        painter.drawPolygon(m_polygon.translated(x, y));
    }

    void setSize(double size)
    {
        Marker::setSize(size);
        precompute(size);
    }

private:

    void precompute(double size)
    {
        m_polygon.clear();
        m_polygon << QPointF(0, -m_size / 2) << QPointF(m_size / 2, 0) << QPointF(0, m_size / 2) << QPointF(-m_size / 2, 0);
    }

    QPolygonF m_polygon;
};

class CrossMarker : public Marker
{
public:

    CrossMarker(double size) : Marker(size) {precompute(size);}

    void paint(QPainter & painter, double x, double y)
    {
        painter.drawLine(m_line1.translated(x,y));
        painter.drawLine(m_line2.translated(x,y));
    }

    void setSize(double size)
    {
        Marker::setSize(size);
        precompute(size);
    }

private:

    void precompute(double size)
    {
        double r = m_size / 2;
        m_line1 = QLineF(QPointF(0, -r), QPointF(0, r));
        m_line2 = QLineF(QPointF(-r, 0), QPointF(r, 0));
    }

    QLineF m_line1;
    QLineF m_line2;
};

class XMarker : public Marker
{
public:

    XMarker(double size) : Marker(size) {precompute(size);}

    void paint(QPainter & painter, double x, double y)
    {
        painter.drawLine(m_line1.translated(x,y));
        painter.drawLine(m_line2.translated(x,y));
    }

    void setSize(double size)
    {
        Marker::setSize(size);
        precompute(size);
    }

private:

    void precompute(double size)
    {
        double r = m_size / 2.82843;
        m_line1 = QLineF(QPointF(-r, -r), QPointF(r, r));
        m_line2 = QLineF(QPointF(r, -r), QPointF(-r, r));
    }

    QLineF m_line1;
    QLineF m_line2;
};

class StarMarker : public Marker
{
public:

    StarMarker(double size) : Marker(size) {precompute(size);}

    void paint(QPainter & painter, double x, double y)
    {
        painter.drawPolygon(m_polygon.translated(x, y));
    }

    void setSize(double size)
    {
        Marker::setSize(size);
        precompute(size);
    }

private:

    void precompute(double size)
    {
        m_polygon.clear();
        static const QPolygonF star(QList<QPointF>() << QPointF(-0.587785,0.809017)
                                                     << QPointF(0,0.4)
                                                     << QPointF(0.587785,0.809017)
                                                     << QPointF(0.380423,0.123607)
                                                     << QPointF(0.951057,-0.309017)
                                                     << QPointF(0.235114,-0.323607)
                                                     << QPointF(0,-1)
                                                     << QPointF(-0.235114,-0.323607)
                                                     << QPointF(-0.951057,-0.309017)
                                                     << QPointF(-0.380423,0.123607));
        m_polygon = QTransform().scale(size / 1.90211, size / 1.90211).map(star);
    }

    QPolygonF m_polygon;
};

class LeftTriangleMarker : public Marker
{
public:

    LeftTriangleMarker(double size) : Marker(size) {precompute(size);}

    void paint(QPainter & painter, double x, double y)
    {
        painter.drawPolygon(m_polygon.translated(x, y));
    }

    void setSize(double size)
    {
        Marker::setSize(size);
        precompute(size);
    }

private:

    void precompute(double size)
    {
        m_polygon.clear();
        double S = size / 2 - sqrt(size * 3.0)/2;
        m_polygon << QPointF(S, -size / 2) << QPointF(-size / 2, 0) << QPointF(S, size / 2);
    }

    QPolygonF m_polygon;
};

class RightTriangleMarker : public Marker
{
public:

    RightTriangleMarker(double size) : Marker(size) {precompute(size);}

    void paint(QPainter & painter, double x, double y)
    {
        painter.drawPolygon(m_polygon.translated(x, y));
    }

    void setSize(double size)
    {
        Marker::setSize(size);
        precompute(size);
    }

private:

    void precompute(double size)
    {
        m_polygon.clear();
        double S = -size / 2 + sqrt(size * 3.0) / 2;
        m_polygon << QPointF(S, -size / 2) << QPointF(size / 2, 0) << QPointF(S, size / 2);
    }

    QPolygonF m_polygon;
};

class UpTriangleMarker : public Marker
{
public:

    UpTriangleMarker(double size) : Marker(size) {precompute(size);}

    void paint(QPainter & painter, double x, double y)
    {
        painter.drawPolygon(m_polygon.translated(x, y));
    }

    void setSize(double size)
    {
        Marker::setSize(size);
        precompute(size);
    }

private:

    void precompute(double size)
    {
        m_polygon.clear();
        double S = size / 2 - sqrt(size * 3.0) / 2;
        m_polygon << QPointF(-size / 2, S) << QPointF(0, -size / 2) << QPointF(size / 2, S);
    }

    QPolygonF m_polygon;
};

class DownTriangleMarker : public Marker
{
public:

    DownTriangleMarker(double size) : Marker(size) {precompute(size);}

    void paint(QPainter & painter, double x, double y)
    {
        painter.drawPolygon(m_polygon.translated(x, y));
    }

    void setSize(double size)
    {
        Marker::setSize(size);
        precompute(size);
    }

private:

    void precompute(double size)
    {
        m_polygon.clear();
        double S = -size / 2 + sqrt(size * 3.0)/2;
        m_polygon << QPointF(-size / 2, S) << QPointF(0, size / 2) << QPointF(size / 2, S);
    }

    QPolygonF m_polygon;
};

} // namespace Plot

#endif // FUZZYDROPLETS_GUI_PLOT_MARKER_H
