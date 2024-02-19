#include "textmarker.h"

#include <QPainter>
#include <QFontMetrics>

namespace Plot
{

TextMarker::TextMarker(ContinuousAxis * xAxis, ContinuousAxis * yAxis, double x, double y, const QString & text)
    : Primitive(),
    m_x(x),
    m_y(y),
    m_xAxis(xAxis),
    m_yAxis(yAxis),
    m_text(text)
{
}

void TextMarker::render(QPainter & painter)
{
    if (this->isVisible() && !m_text.isEmpty()) {

        painter.save();
        painter.setPen(Qt::NoPen);

        apply(painter, m_pen, m_brush, QPalette::Text, QPalette::Base);

        QFont font;
        font.setPixelSize(m_size);
        painter.setFont(font);

        QFontMetrics fm(font);
        auto rect = fm.boundingRect(m_text).adjusted(-3,-3,3,3);

        QRect textRect(m_xAxis->pixel(m_x) - rect.width() / 2, m_yAxis->pixel(m_y) - rect.height() / 2 , rect.width() , rect.height());
        painter.drawRect(textRect);
        painter.drawText(textRect,  Qt::AlignCenter, m_text);
        painter.restore();
    }
}

} // namespace Plot
