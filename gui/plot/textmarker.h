#ifndef FUZZYDROPLETS_GUI_PLOT_TEXTMARKER_H
#define FUZZYDROPLETS_GUI_PLOT_TEXTMARKER_H

#include "primitive.h"
#include "continuousaxis.h"

namespace Plot
{

class TextMarker : public Plot::Primitive
{
public:

    TextMarker(ContinuousAxis * xAxis, ContinuousAxis * yAxis, double x, double y, const QString & text);

    void setValue(double x, double y) {m_x = x; m_y = y;}
    void setX(double x) {m_x = x;}
    void setY(double y) {m_y = y;}
    double x() const {return m_x;}
    double y() const {return m_y;}
    ContinuousAxis * xAxis() const {return m_xAxis;}
    ContinuousAxis * yAxis() const {return m_yAxis;}

    void setText(const QString & text) {m_text = text;}
    const QString & text() {return m_text;}

    void setSize(double size) {m_size = size;}
    double size() const {return m_size;}

    void setPen(const QPen & pen) {m_pen = pen;}
    const QPen & pen() const {return m_pen;}

    void setBrush(const QBrush & brush) {m_brush = brush;}
    const QBrush & brush() const {return m_brush;}

    virtual void render(QPainter & painter);

private:

    double m_x;
    double m_y;
    double m_size {12};
    ContinuousAxis * m_xAxis;
    ContinuousAxis * m_yAxis;
    QPen m_pen {QPen(Qt::black, 1)};
    QBrush m_brush {Qt::white};
    QString m_text;
};

} // namespace Plot

#endif // FUZZYDROPLETS_GUI_PLOT_TEXTMARKER_H
