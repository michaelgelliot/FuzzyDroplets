#ifndef FUZZYDROPLETS_GUI_PLOT_RINGMARKER_H
#define FUZZYDROPLETS_GUI_PLOT_RINGMARKER_H

#include "primitive.h"
#include "continuousaxis.h"

namespace Plot
{

class RingMarker : public Plot::Primitive
{
public:

    RingMarker(ContinuousAxis * xAxis, ContinuousAxis * yAxis, double x, double y);

    void setValue(double x, double y) {m_x = x; m_y = y;}
    void setX(double x) {m_x = x;}
    void setY(double y) {m_y = y;}
    double x() const {return m_x;}
    double y() const {return m_y;}
    void setSize(double size) {m_size = size;}
    double size() const {return m_size;}
    ContinuousAxis * xAxis() const {return m_xAxis;}
    ContinuousAxis * yAxis() const {return m_yAxis;}

    void setPen(const QPen & pen) {m_pen = pen;}
    const QPen & pen() const {return m_pen;}

    virtual void render(QPainter & painter);

private:

    double m_x;
    double m_y;
    double m_size {25};
    ContinuousAxis * m_xAxis;
    ContinuousAxis * m_yAxis;
    QPen m_pen {QPen(Qt::black, 6)};

};

}

#endif // FUZZYDROPLETS_GUI_PLOT_RINGMARKER_H
