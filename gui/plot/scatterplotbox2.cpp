#include "scatterplotbox2.h"
#include "continuousaxis.h"
#include <QWheelEvent>
#include <QPropertyAnimation>

namespace Plot
{

ScatterPlotBox2::ScatterPlotBox2(QWidget * parent)
    : BoxGraphBase (parent)
{
    setLeftAxis(new ContinuousAxis);
    setRightAxis(new ContinuousAxis);
    setTopAxis(new ContinuousAxis);
    setBottomAxis(new ContinuousAxis);
    topAxis()->setVisibility(Axis::MajorTickLabel | Axis::MinorTick | Axis::MediumTick, false);
    rightAxis()->setVisibility(Axis::MajorTickLabel | Axis::MinorTick | Axis::MediumTick, false);

    m_zoomAnimation = new QPropertyAnimation(this, "rangeRectF", this);
}

void ScatterPlotBox2::setRange(double xMin, double xMax, double yMin, double yMax)
{
    ((ContinuousAxis*)bottomAxis())->setRange(xMin, xMax);
    ((ContinuousAxis*)topAxis())->setRange(xMin, xMax);
    ((ContinuousAxis*)leftAxis())->setRange(yMin, yMax);
    ((ContinuousAxis*)rightAxis())->setRange(yMin, yMax);
    updateStaticPrimitives();
    update();
}

void ScatterPlotBox2::setRange(OrthogonalRectangle rect)
{
    setRange(rect.left(), rect.right(), rect.bottom(), rect.top());
}

void ScatterPlotBox2::setRange(QRectF rect)
{
    setRange(rect.left(), rect.right(), rect.top(), rect.bottom());
}

OrthogonalRectangle ScatterPlotBox2::range() const
{
    return OrthogonalRectangle(Point(horizontalAxis()->minimum(), verticalAxis()->minimum()), horizontalAxis()->valueLength(), verticalAxis()->valueLength());
}

QRectF ScatterPlotBox2::rangeRectF() const
{
    return QRectF(horizontalAxis()->minimum(), verticalAxis()->minimum(), horizontalAxis()->valueLength(), verticalAxis()->valueLength());
}

void ScatterPlotBox2::setAbsoluteRange(double xMin, double xMax, double yMin, double yMax)
{
    ((ContinuousAxis*)bottomAxis())->setAbsoluteRange(xMin, xMax);
    ((ContinuousAxis*)topAxis())->setAbsoluteRange(xMin, xMax);
    ((ContinuousAxis*)leftAxis())->setAbsoluteRange(yMin, yMax);
    ((ContinuousAxis*)rightAxis())->setAbsoluteRange(yMin, yMax);
    updateStaticPrimitives();
    update();
}

void ScatterPlotBox2::setupFromDataRange(double xMin, double xMax, double yMin, double yMax)
{
    ((ContinuousAxis*)bottomAxis())->setupFromDataRange(xMin, xMax);
    ((ContinuousAxis*)topAxis())->setupFromDataRange(xMin, xMax);
    ((ContinuousAxis*)leftAxis())->setupFromDataRange(yMin, yMax);
    ((ContinuousAxis*)rightAxis())->setupFromDataRange(yMin, yMax);
    updateStaticPrimitives();
    update();
}

void ScatterPlotBox2::setupFromDataRange(OrthogonalRectangle rect)
{
    ((ContinuousAxis*)bottomAxis())->setupFromDataRange(rect.left(), rect.right());
    ((ContinuousAxis*)topAxis())->setupFromDataRange(rect.left(), rect.right());
    ((ContinuousAxis*)leftAxis())->setupFromDataRange(rect.bottom(), rect.top());
    ((ContinuousAxis*)rightAxis())->setupFromDataRange(rect.bottom(), rect.top());
    updateStaticPrimitives();
    update();
}

OrthogonalRectangle ScatterPlotBox2::visibleBounds() const
{
    return OrthogonalRectangle(Point(horizontalAxis()->minimum(), verticalAxis()->minimum()), horizontalAxis()->maximum() - horizontalAxis()->minimum(), verticalAxis()->maximum() - verticalAxis()->minimum());;
}

OrthogonalRectangle ScatterPlotBox2::constrainedVisibleBounds(const OrthogonalRectangle & visibleBounds) const
{
    OrthogonalRectangle result = visibleBounds;
    if (result.left() < horizontalAxis()->absoluteMinimum()) result.translate(horizontalAxis()->absoluteMinimum() - result.left(), 0);
    if (result.right() > horizontalAxis()->absoluteMaximum()) result.translate(horizontalAxis()->absoluteMaximum() - result.right(), 0);
    if (result.bottom() < verticalAxis()->absoluteMinimum()) result.translate(0, verticalAxis()->absoluteMinimum() - result.bottom());
    if (result.top() > verticalAxis()->absoluteMaximum()) result.translate(0,verticalAxis()->absoluteMaximum() - result.top());
    return result;
}

double ScatterPlotBox2::horizontalUnit() const
{
    return horizontalAxis()->pixelLength() / (horizontalAxis()->maximum() - horizontalAxis()->minimum());
}

double ScatterPlotBox2::verticalUnit() const
{
    return verticalAxis()->pixelLength() / (verticalAxis()->maximum() - verticalAxis()->minimum());
}

void ScatterPlotBox2::wheelEvent (QWheelEvent * e)
{
    if (m_zoomPanEnabled) {
        auto rect = visibleBounds();
        auto originalRect = rect;
        auto shift = e->hasPixelDelta() ? e->pixelDelta() : e->angleDelta() / 8; //todo test for pixel delta and angle delta, my laptop only has angle delta
        if (e->modifiers() == Qt::NoModifier) {
            rect.translate(-2 * shift.x() / horizontalUnit(), 2 * shift.y() / verticalUnit());
        } else if (e->modifiers() == Qt::ControlModifier) {
            auto pos = mapFromGlobal(QCursor::pos()) - viewportRect().topLeft();
            double scale = std::max(0.01, 1.0 - double(shift.y())/100);
            rect.scale(scale, {horizontalAxis()->value(pos.x()), verticalAxis()->value(pos.y())});
        }
        if (rect != originalRect && rect.width() > horizontalAxis()->minorTickUnit() && rect.height() > verticalAxis()->minorTickUnit()) {
            setRange(constrainedVisibleBounds(rect));
            e->accept();
        }
    }
}

void ScatterPlotBox2::resetZoom()
{
    if (m_zoomPanEnabled) {
        if (m_zoomAnimation->state() == QPropertyAnimation::Running)
            m_zoomAnimation->stop();
        double xmin = (horizontalAxis()->absoluteMinimum() == -std::numeric_limits<double>::infinity()) ? horizontalAxis()->minimum() : horizontalAxis()->absoluteMinimum();
        double xmax = (horizontalAxis()->absoluteMaximum() == std::numeric_limits<double>::infinity()) ? horizontalAxis()->maximum() : horizontalAxis()->absoluteMaximum();
        double ymin = (verticalAxis()->absoluteMinimum() == -std::numeric_limits<double>::infinity()) ? verticalAxis()->minimum() : verticalAxis()->absoluteMinimum();
        double ymax = (verticalAxis()->absoluteMaximum() == std::numeric_limits<double>::infinity()) ? verticalAxis()->maximum() : verticalAxis()->absoluteMaximum();
        m_zoomAnimation->setStartValue(rangeRectF());
        m_zoomAnimation->setEndValue(QRectF(xmin, ymin, xmax - xmin, ymax-ymin));
        m_zoomAnimation->setDuration(200);
       m_zoomAnimation->setEasingCurve(QEasingCurve::InOutCubic);
        m_zoomAnimation->start();
    }
}

void ScatterPlotBox2::zoomIn()
{
    if (m_zoomPanEnabled) {
        if (m_zoomAnimation->state() == QPropertyAnimation::Running)
            m_zoomAnimation->stop();
        m_zoomAnimation->setStartValue(rangeRectF());
        m_zoomAnimation->setEndValue(calcZoomedViewport(0.7));
        m_zoomAnimation->setDuration(200);
       m_zoomAnimation->setEasingCurve(QEasingCurve::InOutCubic);
        m_zoomAnimation->start();
    }
}

void ScatterPlotBox2::zoomOut()
{
    if (m_zoomPanEnabled) {
        if (m_zoomAnimation->state() == QPropertyAnimation::Running)
            m_zoomAnimation->stop();
        m_zoomAnimation->setStartValue(rangeRectF());
        m_zoomAnimation->setEndValue(calcZoomedViewport(1.0/0.7));
        m_zoomAnimation->setDuration(300);
//        m_zoomAnimation->setEasingCurve(QEasingCurve::InOutCubic);
        m_zoomAnimation->start();
    }
}

QRectF ScatterPlotBox2::calcZoomedViewport(double factor)
{
    auto viewport = viewportRect();
    QPointF cursorPos = mapFromGlobal(QCursor::pos());
    if (!viewport.contains(cursorPos)) cursorPos = viewport.center();
    double x = horizontalAxis()->value(cursorPos.x() - viewport.left());
    double y = verticalAxis()->value(cursorPos.y() - viewport.top());
    return QRectF(x + (horizontalAxis()->minimum() - x) * factor,
                  y + (verticalAxis()->minimum() - y) * factor,
                  (horizontalAxis()->valueLength()) * factor,
                  (verticalAxis()->valueLength()) * factor);
}


} // nameespace Plot
