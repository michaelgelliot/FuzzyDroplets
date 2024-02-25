#include "dropletgraphwidget.h"
#include "core/colorscheme.h"
#include "gui/generic/commandstack.h"
#include "gui/plot/polygon.h"
#include "pointcloud.h"
#include "core/data.h"
#include "gui/generic/themedicon.h"
#include <QContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QSettings>
#include <execution>

#ifdef Q_OS_MACOS
#include <QtConcurrent>
#endif

DropletGraphWidget::DropletGraphWidget(Data * data, CommandStack * cmdStack, QWidget * parent)
    : ScatterPlotBox2(parent),
     m_data(data),
    m_commandStack(cmdStack),
    m_cloud(new PointCloud(data, horizontalAxis(), verticalAxis()))
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    bottomAxis()->setLabel("Channel 2 Amplitude");
    topAxis()->setLabel("Channel 2 Amplitude");
    leftAxis()->setLabel("Channel 1 Amplitude");
    rightAxis()->setLabel("Channel 1 Amplitude");

    QSettings settings;
    bottomAxis()->setVisibility(Plot::Axis::Label, settings.value("bottomAxisLabel", true).value<bool>());
    bottomAxis()->setVisibility(Plot::Axis::MajorTickLabel, settings.value("bottomAxisMajorTickLabel", true).value<bool>());
    bottomAxis()->setVisibility(Plot::Axis::MajorTick, settings.value("bottomAxisMajorTick", true).value<bool>());
    bottomAxis()->setVisibility(Plot::Axis::MediumTick, settings.value("bottomAxisMediumTick", true).value<bool>());
    bottomAxis()->setVisibility(Plot::Axis::MinorTick, settings.value("bottomAxisMinorTick", true).value<bool>());

    leftAxis()->setVisibility(Plot::Axis::Label, settings.value("leftAxisLabel", true).value<bool>());
    leftAxis()->setVisibility(Plot::Axis::MajorTickLabel, settings.value("leftAxisMajorTickLabel", true).value<bool>());
    leftAxis()->setVisibility(Plot::Axis::MajorTick, settings.value("leftAxisMajorTick", true).value<bool>());
    leftAxis()->setVisibility(Plot::Axis::MediumTick, settings.value("leftAxisMediumTick", true).value<bool>());
    leftAxis()->setVisibility(Plot::Axis::MinorTick, settings.value("leftAxisMinorTick", true).value<bool>());

    topAxis()->setVisibility(Plot::Axis::Label, settings.value("topAxisLabel", false).value<bool>());
    topAxis()->setVisibility(Plot::Axis::MajorTickLabel, settings.value("topAxisMajorTickLabel", false).value<bool>());
    topAxis()->setVisibility(Plot::Axis::MajorTick, settings.value("topAxisMajorTick", true).value<bool>());
    topAxis()->setVisibility(Plot::Axis::MediumTick, settings.value("topAxisMediumTick", true).value<bool>());
    topAxis()->setVisibility(Plot::Axis::MinorTick, settings.value("topAxisMinorTick", false).value<bool>());

    rightAxis()->setVisibility(Plot::Axis::Label, settings.value("rightAxisLabel", false).value<bool>());
    rightAxis()->setVisibility(Plot::Axis::MajorTickLabel, settings.value("rightAxisMajorTickLabel", false).value<bool>());
    rightAxis()->setVisibility(Plot::Axis::MajorTick, settings.value("rightAxisMajorTick", true).value<bool>());
    rightAxis()->setVisibility(Plot::Axis::MediumTick, settings.value("rightAxisMediumTick", true).value<bool>());
    rightAxis()->setVisibility(Plot::Axis::MinorTick, settings.value("rightAxisMinorTick", false).value<bool>());

    m_cloud->setScaleFactor(settings.value("graphWidgetScaleFactor", true).toBool() ? 0.15 : 0);
    m_cloud->setBaseSize(settings.value("graphWidgetMarkerSize", 3).toDouble());
    addStaticPrimitive(m_cloud);

    connect(m_data, &Data::samplesAdded, this, &DropletGraphWidget::samplesAdded);
    connect(m_data, &Data::selectedSamplesChanged, this, &DropletGraphWidget::selectedSamplesChanged);
    connect(m_data, &Data::fullRepaint, this, &DropletGraphWidget::repaintEntireGraph);
    connect(m_data, &Data::colorZOrderChanged, this, &DropletGraphWidget::repaintEntireGraph);
    connect(m_data, &Data::colorCountChanged, this, &DropletGraphWidget::updateConvexHullCount);
    connect(m_data, &Data::colorCountChanged, this, &DropletGraphWidget::updateConvexHullColors);
    connect(m_data, &Data::designChanged, this, &DropletGraphWidget::updateConvexHullColors);
}

void DropletGraphWidget::samplesAdded()
{
    setupFromDataRange(m_data->bounds());
    recalculateLayout();
    m_cloud->setQuadTree(m_data->quadTree());
    updateStaticPrimitives();
    update();
}

void DropletGraphWidget::selectedSamplesChanged()
{
    updateStaticPrimitives();
    updateConvexHulls();
    update();
}

void DropletGraphWidget::repaintEntireGraph()
{
    updateStaticPrimitives();
    update();
}

size_t DropletGraphWidget::markerFromMousePos(const QPoint & pos) const
{
    auto viewport = viewportRect();
    Point value (horizontalAxis()->value(pos.x() - viewport.left()), verticalAxis()->value(pos.y() - viewport.top()));
    double xScale = horizontalAxis()->valueLength() / horizontalAxis()->pixelLength();
    double yScale = verticalAxis()->valueLength() / verticalAxis()->pixelLength();
    auto i = m_data->nearestNeighbourInSelection(value, xScale, yScale);
    if (i.first != -1) {
        auto m = m_cloud->maxMarkerSize();
        double squaredPixelDistance = pow(horizontalAxis()->pixel(m_data->point(i.first).x()) - (pos.x() - viewport.left()),2) + pow(verticalAxis()->pixel(m_data->point(i.first).y()) - (pos.y() - viewport.top()),2);
        if (squaredPixelDistance < std::max(2.0, pow(2 * m,2)))
            return (int)i.first;
    }
    return -1;
}

void DropletGraphWidget::contextMenuEvent(QContextMenuEvent * e)
{
    size_t nearbyMarker = markerFromMousePos(e->pos());
    size_t colorId = (nearbyMarker == -1) ?  -1 : m_data->fuzzyColor(nearbyMarker).dominantComponent();

    QMenu menu(this);
    QAction * front {nullptr};
    QAction * back {nullptr};
    QAction * forward {nullptr};
    QAction * backward {nullptr};
    auto h = menu.addAction("View Convex Hulls");
    h->setCheckable(true);
    h->setChecked(m_paintConvexHulls);
    h->setEnabled(m_data->selectedSamples().size() > 0);

    menu.addSeparator();
    auto select = menu.addAction("Select Sample");
    if (nearbyMarker == -1) select->setEnabled(false);

    if (colorId >= 0) {
        QPixmap p(50,50);
        p.fill(Qt::transparent);
        QPainter paint(&p);
        paint.setPen(Qt::NoPen);
        paint.setBrush(QColor::fromRgb(m_data->colorScheme()->color(colorId)));
        paint.drawEllipse(p.rect());
        paint.end();
        menu.addSeparator();
        auto arrange = menu.addMenu(p, "Arrange");
        front = arrange->addAction(themedIcon(":/bringToFront"), "Bring to Front");
        front->setEnabled(m_data->colorZOrder(colorId) != m_data->colorComponentCount() - 1);
        forward = arrange->addAction("Bring Forward");
        forward->setEnabled(m_data->colorZOrder(colorId) != m_data->colorComponentCount() - 1);
        arrange->addSeparator();
        back = arrange->addAction(themedIcon(":/sendToBack"), "Send to Back");
        back->setEnabled(m_data->colorZOrder(colorId) != 0);
        backward = arrange->addAction("Send Backward");
        backward->setEnabled(m_data->colorZOrder(colorId) != 0);
        if (nearbyMarker == -1) arrange->setEnabled(false);
    }
    auto a = menu.exec(e->globalPos());

    if (a == select) {
        size_t i = 0;
        while (i < m_data->sampleCount() && nearbyMarker > m_data->sampleIndices(i)[1]) {
            ++i;
        }
        m_data->setSelectedSamples(std::vector<size_t>(1, i));
    } else if (front && a == front) {
        m_commandStack->add(new SetZOrderCommand(m_data, colorId, m_data->colorZOrder(colorId), m_data->colorComponentCount()));
    } else if (back && a == back) {
        m_commandStack->add(new SetZOrderCommand(m_data, colorId, m_data->colorZOrder(colorId), 0));
    } else if (forward && a == forward) {
        m_commandStack->add(new SetZOrderCommand(m_data, colorId, m_data->colorZOrder(colorId), m_data->colorZOrder(colorId) + 1));
    } else if (backward && a == backward) {
        m_commandStack->add(new SetZOrderCommand(m_data, colorId, m_data->colorZOrder(colorId), m_data->colorZOrder(colorId) - 1));
    } else if (a == h) {
        setConvexHullsVisible(!m_paintConvexHulls);
    }
}

DropletGraphWidget::SetZOrderCommand::SetZOrderCommand(Data * data, size_t colorId, size_t oldZ, size_t newZ)
    : m_data(data),
    m_colorId(colorId),
    m_oldZ(oldZ),
    m_newZ(newZ)
{
}

void DropletGraphWidget::SetZOrderCommand::redo()
{
    m_data->setColorZOrder(m_colorId, m_newZ);
}

void DropletGraphWidget::SetZOrderCommand::undo()
{
    m_data->setColorZOrder(m_colorId, m_oldZ);
}

void DropletGraphWidget::parallelWorkStarted()
{
    setEnabled(false);
}

void DropletGraphWidget::parallelWorkFinished()
{
    setEnabled(true);
}

void DropletGraphWidget::setConvexHullsVisible(bool b)
{
    if (b != m_paintConvexHulls) {
        m_paintConvexHulls = b;
        if (b) {
            updateConvexHulls();
        }
        for (auto & hull : m_convexHulls) {
            hull->setVisibility(b);
        }
        update();
    }
}

void DropletGraphWidget::updateConvexHullCount()
{
    if (m_data->colorComponentCount() > (size_t)m_convexHulls.size()) {
        for (size_t i = m_convexHulls.size() + 1; i < m_data->colorComponentCount() ; ++i) {
            Plot::Polygon * p = new Plot::Polygon(horizontalAxis(), verticalAxis());
            p->setAutoColor(false);
            QColor color = m_data->colorScheme()->color(i);
            color.setAlpha(125);
            p->setPen(QPen(color, 1));
            color.setAlpha(30);
            p->setBrush(color);
            p->setVisibility(true);
            addDynamicPrimitive(p);
            m_convexHulls.append(p);
        }
        if (m_paintConvexHulls)
            updateConvexHulls();
    } else if (m_data->colorComponentCount() < m_convexHulls.size()) {
        m_convexHulls.resize(m_data->colorComponentCount() - 1);
    }
}

void DropletGraphWidget::updateConvexHullColors()
{
    for (size_t i = 0; i < m_convexHulls.size(); ++i) {
        QColor color = m_data->colorScheme()->color(i+1);
        color.setAlpha(125);
        m_convexHulls[i]->setPen(QPen(color, 1));
        color.setAlpha(30);
        m_convexHulls[i]->setBrush(color);
    }
    update();
}

void DropletGraphWidget::updateConvexHulls()
{
    if (m_paintConvexHulls) {
        if (m_data->selectedPointCount() > 0) {
            double limit = 1.0 / m_data->colorComponentCount();
            QList<int> io(m_convexHulls.size());
            std::iota(io.begin(), io.end(), 1);
#ifndef Q_OS_MACOS
            std::for_each(std::execution::par, io.begin(), io.end(), [&](int i) {
#else
            QtConcurrent::blockingMap(io.begin(), io.end(), [&](int i) {
#endif
                std::vector<Point> cols;
                for (auto j = 0; j < m_data->points().size(); ++j)
                    if (m_data->isSelected(j) && m_data->fuzzyColor(j).weight(i) >= limit)
                        cols.push_back(m_data->point(j));
                auto poly = Polygon::convexHull(cols);
                QPolygonF qp;
                for (const auto & point : poly.points())
                    qp << QPointF(point.x(), point.y());
                m_convexHulls[i - 1]->setPolygon(qp);
            });
        } else {
            for (auto & p : m_convexHulls) {
                p->setPolygon(QPolygonF());
            }
        }
    }
}

void DropletGraphWidget::setMarkerSize(int i)
{
    QSettings settings;
    settings.setValue("graphWidgetMarkerSize", i);
    m_cloud->setBaseSize(i);
    updateStaticPrimitives();
    update();
}

void DropletGraphWidget::setScaleFactor(double d)
{
    m_cloud->setScaleFactor(d);
    updateStaticPrimitives();
    update();
}
