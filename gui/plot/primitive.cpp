#include "primitive.h"
#include <QPainter>
#include <QGuiApplication>

namespace Plot
{

void Primitive::apply(QPainter & painter, const QPen & pen, QPalette::ColorRole cr)
{
    if (autoColor()) {
        QPen p = pen;
        p.setColor(qGuiApp->palette().color(cr));
        painter.setPen(p);
    } else {
        painter.setPen(pen);
    }
}

void Primitive::apply(QPainter & painter, const QBrush & brush, QPalette::ColorRole cr)
{
    if (autoColor()) {
        QBrush b = brush;
        b.setColor(qGuiApp->palette().color(cr));
        painter.setBrush(b);
    } else {
        painter.setBrush(brush);
    }
}

void Primitive::apply(QPainter & painter, const QPen & pen, const QBrush & brush, QPalette::ColorRole penCr, QPalette::ColorRole brushCr)
{
    if (autoColor()) {
        QPen p = pen;
        QBrush b = brush;
        p.setColor(qGuiApp->palette().color(penCr));
        b.setColor(qGuiApp->palette().color(brushCr));
        painter.setPen(p);
        painter.setBrush(b);
    } else {
        painter.setPen(pen);
        painter.setBrush(brush);
    }
}

} // namespace Plot
