#include "boxgraphbase.h"
#include "axis.h"
#include <QPainter>
#include <QMouseEvent>
#include <QStyleHints>
#include <QGuiApplication>
#include <QPainterPath>
#include <QSvgGenerator>
#include <QPaintEngine>

namespace Plot
{

BoxGraphBase::BoxGraphBase(QWidget *parent)
    : QWidget{parent}
{
    setMouseTracking(true);
    m_staticPixmap = QPixmap(2,2);
    connect(qGuiApp->styleHints(), &QStyleHints::colorSchemeChanged, this, &BoxGraphBase::guiColorSchemeChanged, Qt::QueuedConnection);
}

BoxGraphBase::~BoxGraphBase()
{
    for (auto * primitive : m_staticPrimitives)
        delete primitive;
    for (auto * primitive : m_dynamicPrimitives)
        delete primitive;
    delete m_leftAxis;
    delete m_rightAxis;
    delete m_topAxis;
    delete m_bottomAxis;
}

void BoxGraphBase::setLeftAxis(Axis * axis)
{
    delete m_leftAxis;
    m_leftAxis = axis;
    if (axis) {
        axis->setOrientation(Qt::Vertical);
        axis->setAlignment(Axis::Line | Axis::Label | Axis::MajorTick | Axis::MediumTick | Axis::MinorTick | Axis::MajorTickLabel | Axis::MediumTickLabel | Axis::MinorTickLabel, Axis::AlignLeft);
        QFont f;
        f.setBold(true);
        axis->setLabelFont(f);
        recalculateLayout();
        update();
    }
}

void BoxGraphBase::setRightAxis(Axis * axis)
{
    delete m_rightAxis;
    m_rightAxis = axis;
    if (axis) {
        axis->setOrientation(Qt::Vertical);
        axis->setAlignment(Axis::Line | Axis::Label | Axis::MajorTick | Axis::MediumTick | Axis::MinorTick | Axis::MajorTickLabel | Axis::MediumTickLabel | Axis::MinorTickLabel, Axis::AlignRight);
        QFont f;
        f.setBold(true);
        axis->setLabelFont(f);
        recalculateLayout();
        update();
    }
}

void BoxGraphBase::setTopAxis(Axis * axis)
{
    delete m_topAxis;
    m_topAxis = axis;
    if (axis) {
        axis->setOrientation(Qt::Horizontal);
        axis->setAlignment(Axis::Line | Axis::Label | Axis::MajorTick | Axis::MediumTick | Axis::MinorTick | Axis::MajorTickLabel | Axis::MediumTickLabel | Axis::MinorTickLabel, Axis::AlignRight);
        QFont f;
        f.setBold(true);
        axis->setLabelFont(f);
        recalculateLayout();
        update();
    }
}

void BoxGraphBase::setBottomAxis(Axis * axis)
{
    delete m_bottomAxis;
    m_bottomAxis = axis;
    if (axis) {
        axis->setOrientation(Qt::Horizontal);
        axis->setAlignment(Axis::Line | Axis::Label | Axis::MajorTick | Axis::MediumTick | Axis::MinorTick | Axis::MajorTickLabel | Axis::MediumTickLabel | Axis::MinorTickLabel, Axis::AlignLeft);
        QFont f;
        f.setBold(true);
        axis->setLabelFont(f);
        recalculateLayout();
        update();
    }
}

void BoxGraphBase::removeStaticPrimitive(Primitive * primitive)
{
    m_staticPrimitives.removeAll(primitive);
    updateStaticPrimitives();
}

void BoxGraphBase::removeDynamicPrimitive(Primitive * primitive)
{
    m_dynamicPrimitives.removeAll(primitive);
}

QRectF BoxGraphBase::viewportRect() const
{
    return QRectF(QPointF(m_padding + (m_leftAxis ? m_leftAxis->leftOffset() : 0), m_padding + (m_topAxis ? m_topAxis->rightOffset() : 0)),
                  QPointF(width() - m_padding - (m_rightAxis ? m_rightAxis->rightOffset() : 0),  height() - m_padding - (m_bottomAxis ? m_bottomAxis->leftOffset() : 0)));
}

void BoxGraphBase::resizeEvent(QResizeEvent * e)
{
    recalculateLayout();
    m_staticPixmap = QPixmap(width() * devicePixelRatio() + 2, height() * devicePixelRatio() + 2);
    m_staticPixmap.setDevicePixelRatio(devicePixelRatio());
    updateStaticPrimitives();
}

void BoxGraphBase::paintEvent(QPaintEvent * e)
{
    QPainter painter(this);
    painter.setClipRect(e->rect());
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    render(painter);
}

void BoxGraphBase::updateStaticPrimitives(const QRectF & clipRect)
{
    QPainter paint(&m_staticPixmap);
    if (clipRect.isValid())
        paint.setClipRect(clipRect);
    else
        paint.setClipRect(QRect(0,0,(int)m_staticPixmap.width(), (int)m_staticPixmap.height()));
    paint.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    auto viewport = viewportRect();
    auto oldClip = paint.clipRegion();
    auto clip = oldClip.intersected(viewport.toRect()); //todo

    paint.setPen(Qt::NoPen);
    apply(paint, m_backgroundBrush, QPalette::Base);
    paint.drawRect(rect());

    apply(paint, m_viewportBrush, QPalette::Base);
    paint.drawRect(viewport);

    for (int i = 0; i < m_staticPrimitives.size(); ++i) {
        if (m_staticPrimitives[i]->isVisible()) {
            if (m_staticPrimitives[i]->clipToViewport()) {
                paint.setClipRegion(clip);
            } else {
                paint.setClipRegion(oldClip);
            }
            paint.translate(viewport.topLeft());
            m_staticPrimitives[i]->render(paint);
            paint.translate(-viewport.topLeft());
        }
    }
}

void BoxGraphBase::render(QPainter & paint, bool highQualityOutput)
{
    paint.save();

    if (highQualityOutput) {

        paint.setClipRect(QRect(0, 0, m_staticPixmap.width(), m_staticPixmap.height()));

        paint.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        auto viewport = viewportRect();
        auto oldClip = paint.clipRegion();
        auto clip = oldClip.intersected(viewport.toRect()); //todo

        paint.setPen(Qt::NoPen);
        apply(paint, m_backgroundBrush, QPalette::Base);
        paint.drawRect(rect());

        apply(paint, m_viewportBrush, QPalette::Base);
        paint.drawRect(viewport);

        for (int i = 0; i < m_staticPrimitives.size(); ++i) {
            if (m_staticPrimitives[i]->isVisible()) {
                if (m_staticPrimitives[i]->clipToViewport()) {
                    paint.setClipRegion(clip);
                } else {
                    paint.setClipRegion(oldClip);
                }
                paint.translate(viewport.topLeft());
                m_staticPrimitives[i]->render(paint, highQualityOutput);
                paint.translate(-viewport.topLeft());
            }
        }

        paint.setClipRegion(oldClip);

    } else {

        paint.drawPixmap(0,0,m_staticPixmap);

    }

    auto viewport = viewportRect();
    auto oldClip = paint.clipRegion();
    auto clip = oldClip.intersected(viewport.toRect()); //todo

    for (int i = 0; i < m_dynamicPrimitives.size(); ++i) {
        if (m_dynamicPrimitives[i]->isVisible()) {
            if (m_dynamicPrimitives[i]->clipToViewport()) {
                paint.setClipRegion(clip);
            } else {
                paint.setClipRegion(oldClip);
            }
            paint.translate(viewport.topLeft());
            m_dynamicPrimitives[i]->render(paint, highQualityOutput);
            paint.translate(-viewport.topLeft());
        }
    }

    paint.setClipRegion(oldClip);

    if (highQualityOutput) {
        // Qt SVG does not support clipping so before drawing axes we blank out everything outside the graph viewport
        QRegion region = QRegion(rect()).subtracted(viewportRect().toRect());
        paint.setBrush(palette().base());
        paint.setPen(Qt::NoPen);
        for (auto & rect : region) {
            paint.drawRect(rect);
        }
    }

    if (m_leftAxis) {
        paint.translate(viewport.bottomLeft());
        m_leftAxis->render(paint, highQualityOutput);
        paint.translate(-viewport.bottomLeft());
    }
    if (m_bottomAxis) {
        paint.translate(viewport.bottomLeft());
        m_bottomAxis->render(paint, highQualityOutput);
        paint.translate(-viewport.bottomLeft());
    }
    if (m_topAxis) {
        paint.translate(viewport.topLeft());
        m_topAxis->render(paint, highQualityOutput);
        paint.translate(-viewport.topLeft());
    }
    if (m_rightAxis) {
        paint.translate(viewport.bottomRight());
        m_rightAxis->render(paint, highQualityOutput);
        paint.translate(-viewport.bottomRight());
    }

    paint.restore();
}

void BoxGraphBase::recalculateLayout()
{
    auto viewport = viewportRect();
    if (m_leftAxis) m_leftAxis->setPixelLength(viewport.height());
    if (m_rightAxis) m_rightAxis->setPixelLength(viewport.height());
    if (m_topAxis) m_topAxis->setPixelLength(viewport.width());
    if (m_bottomAxis) m_bottomAxis->setPixelLength(viewport.width());
    updateStaticPrimitives();
}

void BoxGraphBase::addStaticPrimitive(Primitive * primitive)
{
    m_staticPrimitives.push_back(primitive);
    updateStaticPrimitives();
    update();
}

void BoxGraphBase::addDynamicPrimitive(Primitive * primitive)
{
    m_dynamicPrimitives.push_back(primitive);
    update();
}

void BoxGraphBase::guiColorSchemeChanged()
{
    m_leftAxis->updateLabelPixmap(devicePixelRatio());
    m_rightAxis->updateLabelPixmap(devicePixelRatio());
    m_topAxis->updateLabelPixmap(devicePixelRatio());
    m_bottomAxis->updateLabelPixmap(devicePixelRatio());
    updateStaticPrimitives();
    update();
}

void BoxGraphBase::exportSvg(const std::string & path)
{
    QList<QFont> fonts;
    fonts.push_back(m_leftAxis->labelFont());
    fonts.push_back(m_leftAxis->majorTickLabelFont());
    fonts.push_back(m_leftAxis->mediumTickLabelFont());
    fonts.push_back(m_leftAxis->minorTickLabelFont());
    fonts.push_back(m_rightAxis->labelFont());
    fonts.push_back(m_rightAxis->majorTickLabelFont());
    fonts.push_back(m_rightAxis->mediumTickLabelFont());
    fonts.push_back(m_rightAxis->minorTickLabelFont());
    fonts.push_back(m_topAxis->labelFont());
    fonts.push_back(m_topAxis->majorTickLabelFont());
    fonts.push_back(m_topAxis->mediumTickLabelFont());
    fonts.push_back(m_topAxis->minorTickLabelFont());
    fonts.push_back(m_bottomAxis->labelFont());
    fonts.push_back(m_bottomAxis->majorTickLabelFont());
    fonts.push_back(m_bottomAxis->mediumTickLabelFont());
    fonts.push_back(m_bottomAxis->minorTickLabelFont());

    for (auto & font : fonts) {
        font.setPointSizeF(font.pointSizeF() * devicePixelRatio());
    }

    m_leftAxis->setLabelFont(fonts[0]);
    m_leftAxis->setMajorTickLabelFont(fonts[1]);
    m_leftAxis->setMediumTickLabelFont(fonts[2]);
    m_leftAxis->setMinorTickLabelFont(fonts[3]);
    m_rightAxis->setLabelFont(fonts[4]);
    m_rightAxis->setMajorTickLabelFont(fonts[5]);
    m_rightAxis->setMediumTickLabelFont(fonts[6]);
    m_rightAxis->setMinorTickLabelFont(fonts[7]);
    m_topAxis->setLabelFont(fonts[8]);
    m_topAxis->setMajorTickLabelFont(fonts[9]);
    m_topAxis->setMediumTickLabelFont(fonts[10]);
    m_topAxis->setMinorTickLabelFont(fonts[11]);
    m_bottomAxis->setLabelFont(fonts[12]);
    m_bottomAxis->setMajorTickLabelFont(fonts[13]);
    m_bottomAxis->setMediumTickLabelFont(fonts[14]);
    m_bottomAxis->setMinorTickLabelFont(fonts[15]);

    recalculateLayout();

    QSvgGenerator generator;
    generator.setFileName(QString::fromStdString(path));
    generator.setSize(QSize(m_staticPixmap.width() / devicePixelRatio(), m_staticPixmap.height() / devicePixelRatio()));
    generator.setViewBox(QRect(0, 0, m_staticPixmap.width() / devicePixelRatio(), m_staticPixmap.height() / devicePixelRatio()));
    generator.setTitle(tr("Droplets"));

    QPainter paint;
    paint.begin(&generator);
    paint.setRenderHint(QPainter::Antialiasing);
    render(paint, true);
    paint.end();

    for (auto & font : fonts) {
        font.setPointSizeF(font.pointSizeF() / devicePixelRatio());
    }

    m_leftAxis->setLabelFont(fonts[0]);
    m_leftAxis->setMajorTickLabelFont(fonts[1]);
    m_leftAxis->setMediumTickLabelFont(fonts[2]);
    m_leftAxis->setMinorTickLabelFont(fonts[3]);
    m_rightAxis->setLabelFont(fonts[4]);
    m_rightAxis->setMajorTickLabelFont(fonts[5]);
    m_rightAxis->setMediumTickLabelFont(fonts[6]);
    m_rightAxis->setMinorTickLabelFont(fonts[7]);
    m_topAxis->setLabelFont(fonts[8]);
    m_topAxis->setMajorTickLabelFont(fonts[9]);
    m_topAxis->setMediumTickLabelFont(fonts[10]);
    m_topAxis->setMinorTickLabelFont(fonts[11]);
    m_bottomAxis->setLabelFont(fonts[12]);
    m_bottomAxis->setMajorTickLabelFont(fonts[13]);
    m_bottomAxis->setMediumTickLabelFont(fonts[14]);
    m_bottomAxis->setMinorTickLabelFont(fonts[15]);

    recalculateLayout();
}

QImage BoxGraphBase::image(double scale)
{
    QImage image(scale * width(), scale * height(), QImage::Format_ARGB32);
    QPainter painter(&image);
    painter.scale(scale, scale);
    painter.setClipRect(rect());
    render(painter, true);
    painter.end();
    return image;
}

} // namespace Plot
