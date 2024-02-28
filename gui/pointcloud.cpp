#include "pointcloud.h"
#include <QPainter>
#include <QPaintDevice>
#include <QPaintEngine>
#include <execution>
#include <ranges>
#include "core/colorscheme.h"
#include "core/data.h"
#include "core/quadtree.h"
#include "qpainterpath.h"
#include <QtGlobal>

#ifdef Q_OS_MACOS
#include <QtConcurrent>
#endif

PointCloud::PointCloud(Data * data, Plot::ContinuousAxis * xAxis, Plot::ContinuousAxis * yAxis)
    : Primitive(), m_xAxis(xAxis), m_yAxis(yAxis), m_data(data)
{
}

void PointCloud::render(QPainter & painter, bool highQualityOutput)
{
    double dpr = painter.device()->devicePixelRatio();
    viewWidth = m_xAxis->pixelLength() * dpr + 1;
    viewHeight = m_yAxis->pixelLength() * dpr + 1;
    if (viewWidth <= 0 || viewHeight <= 0) return;

    int S =  m_baseSize * (1.0 + m_scaleFactor * (m_xAxis->absoluteValueLength() / m_xAxis->valueLength() - 1.0));
    int S2 = S/2 + ((S % 2) == 1);
    S /= 2;
    auto f = painter.clipBoundingRect().adjusted(-S, -S, S, S);

    OrthogonalRectangle rect(Point(m_xAxis->value(f.left()), m_yAxis->value(f.bottom())), Point(m_xAxis->value(f.right()), m_yAxis->value(f.top())));
    QList<size_t> items = m_data->quadTree()->rectangleSearch(m_data->points(), rect, [&](size_t i){return m_data->isSelected(i);});

    m_pixelData.resize(viewWidth * viewHeight);

#ifndef Q_OS_MACOS
    std::fill(std::execution::par,m_pixelData.begin(), m_pixelData.end(), Color::named::transparent);
#else
    QtConcurrent::blockingMap(m_pixelData.begin(), m_pixelData.end(), [](QRgb & color) {color = Color::named::transparent;});
#endif

    const double limit = 1.0/m_data->colorComponentCount() - 0.000001;

    if (highQualityOutput) {

        double dS = 1.3 * (m_baseSize * (1.0 + m_scaleFactor * (m_xAxis->absoluteValueLength() / m_xAxis->valueLength() - 1.0)));
        double dS2 = 1.3 * (dS/2);

        for (auto col : m_data->colorZOrder()) {
            std::for_each(items.begin(), items.end(), [&](const auto & p) { // can't be parallel
                if (m_data->fuzzyColor(p).dominantComponent() == col) { // it's not clear if this will be slower than just asking if the weight of col > 1.0/m_data->colorComponentCount() [which risks repainting each droplet multiple times when there is a lot of fuzziness in the graph; alternative, keep track of which have been painted and and only paint once per droplet?]
                    auto pt = m_data->point(p);
                    if (m_roundSVGMarkers) {
                        painter.setBrush(QColor::fromRgb(m_data->rgba(p)));
                        painter.drawEllipse(QRectF(m_xAxis->pixel(pt.x()) * dpr - dS, m_yAxis->pixel(pt.y()) * dpr - dS, dS2, dS2));
                    } else {
                        painter.fillRect(QRectF(m_xAxis->pixel(pt.x()) * dpr - dS, m_yAxis->pixel(pt.y()) * dpr - dS, dS2, dS2), QColor::fromRgb(m_data->rgba(p)));
                    }
                }
            });
        }

    } else {

        for (auto col : m_data->colorZOrder()) {
#ifndef Q_OS_MACOS
            std::for_each(std::execution::par, items.begin(), items.end(), [&](const auto & p) {
#else
            QtConcurrent::blockingMap(items.begin(), items.end(), [&](auto & p) {
#endif
                //if (m_data->fuzzyColor(p).dominantComponent() == col) {
                if (m_data->fuzzyColor(p).weight(col) >= limit) {
                    auto pt = m_data->point(p);
                    drawSquare(S, S2, m_xAxis->pixel(pt.x()) * dpr, m_yAxis->pixel(pt.y()) * dpr, m_data->rgba(p), m_pixelData);
                }
            });
        }

        QImage img(reinterpret_cast<uchar*>(m_pixelData.data()), viewWidth, viewHeight, QImage::Format_ARGB32);
        img.setDevicePixelRatio(painter.device()->devicePixelRatio());
        painter.drawImage(0, 0, img);
    }
}

void PointCloud::drawSquare(int S, int S2, size_t x, size_t y, QRgb rgb, std::vector<QRgb> & pixels)
{
    for (int Y = std::max(0, (int)(y - S)); Y < std::min(viewHeight - 1, (int)(y + S2)); ++Y) {
        for (int X = std::max(0, (int)(x - S)); X < std::min(viewWidth - 1, (int)(x + S2)); ++X) {
            pixels[Y * viewWidth + X] = rgb;
        }
    }
}
