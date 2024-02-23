#include "emclustermethodwidget.h"
#include "core/centroids.h"
#include "gui/dropletgraphwidget.h"
#include "gui/generic/themedicon.h"
#include "gui/plot/ringmarker.h"
#include "emworker.h"
#include <QFormLayout>
#include <QSpinBox>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QMouseEvent>
#include <QThread>

EMClusterMethodWidget::EMClusterMethodWidget(Data * data, DropletGraphWidget * graph, QWidget *parent)
    : ClusterMethodWidget{parent},
    m_data(data),
    m_graph(graph)
{
    QFormLayout * form = new QFormLayout;
    setLayout(form);
    form->setContentsMargins(0,0,0,0);

    m_numClusters = new QSpinBox(this);
    m_numClusters->setRange(1, 16);
    m_numClusters->setValue(4);
    connect(m_numClusters, &QSpinBox::valueChanged, this, &EMClusterMethodWidget::numClustersChanged);
    form->addRow("Number of Clusters", m_numClusters);

    m_replicates = new QSpinBox(this);
    m_replicates->setRange(1, 1000);
    m_replicates->setValue(20);
    m_replicates->setSingleStep(10);
    form->addRow("Replicates", m_replicates);

    m_maxIters = new QSpinBox(this);
    m_maxIters->setRange(10, 1000);
    m_maxIters->setValue(100);
    m_maxIters->setSingleStep(50);
    form->addRow("Max Iterations", m_maxIters);

    QGroupBox * initBox = new QGroupBox("Centroid Initialization", this);
    m_initBoxLayout = new QFormLayout;
    initBox->setLayout(m_initBoxLayout);
#ifndef Q_OS_MACOS
    initBox->setFlat(true);
#endif

    m_centroidInit = new QComboBox(this);
    m_centroidInit->addItem("Current Colors",   CentroidInitialization::CurrentColors);
    m_centroidInit->addItem("Farthest Point",   CentroidInitialization::FarthestPoint);
    m_centroidInit->addItem("k-Means++",        CentroidInitialization::KMeansPP);
    m_centroidInit->addItem("Random Centroids", CentroidInitialization::RandomCentroids);
    m_centroidInit->addItem("Random Medoids",   CentroidInitialization::RandomMedoids);
    m_centroidInit->addItem("Set Manually",     CentroidInitialization::CustomCentroids);
    m_centroidInit->setCurrentIndex(1);
    connect(m_centroidInit, &QComboBox::currentIndexChanged, this, &EMClusterMethodWidget::initMethodChanged);
    m_initBoxLayout->addRow("Method", m_centroidInit);

    m_sampleSize = new QSpinBox(this);
    m_sampleSize->setRange(0, 20000);
    m_sampleSize->setValue(2000);
    m_sampleSize->setSingleStep(1000);
    m_initBoxLayout->addRow("Sample Size", m_sampleSize);

    m_reinitialize = new QPushButton(themedIcon(":/refresh"), "Reinitialize", this);
    connect(m_reinitialize, &QPushButton::pressed, this, &EMClusterMethodWidget::initializeCentroidMarkers);
    m_initBoxLayout->addWidget(m_reinitialize);
    m_initBoxLayout->setRowVisible(m_reinitialize, false);

    form->addRow(initBox);

    connect(m_data, &Data::colorCountChanged, this, &EMClusterMethodWidget::dataColorCountChanged);
}

EMClusterMethodWidget::~EMClusterMethodWidget()
{
    if (workerThread) {
        workerThread->quit();
        workerThread->wait();
        delete workerThread;
    }
}

int EMClusterMethodWidget::numClusters() const
{
    return m_numClusters->value();
}

int EMClusterMethodWidget::numReplicates() const
{
    return m_replicates->value();
}

int EMClusterMethodWidget::maxIters() const
{
    return m_maxIters->value();
}

int EMClusterMethodWidget::sampleSize() const
{
    return m_sampleSize->value();
}

CentroidInitialization EMClusterMethodWidget::centroidInitializationMethod() const
{
    return m_centroidInit->itemData(m_centroidInit->currentIndex()).value<CentroidInitialization>();
}

void EMClusterMethodWidget::activate()
{
    m_numClusters->setRange(1, m_data->colorComponentCount() - 1);
    m_numClusters->setValue(m_data->colorComponentCount() - 1);
    for (auto * marker : m_markers)
        marker->setVisibility(true);
    m_graph->update();
    m_graph->installEventFilter(this);
}

void EMClusterMethodWidget::deactivate()
{
    for (auto * marker : m_markers)
        marker->setVisibility(false);
    m_graph->setCursor(Qt::ArrowCursor);
    m_graph->removeEventFilter(this);
    m_graph->update();
}

void EMClusterMethodWidget::dataColorCountChanged()
{
    m_numClusters->setRange(1, m_data->colorComponentCount() - 1);
    m_numClusters->setValue(m_data->colorComponentCount() - 1);
}

bool EMClusterMethodWidget::eventFilter(QObject *obj, QEvent * e)
{
    if (m_centroidInit->itemData(m_centroidInit->currentIndex()).value<CentroidInitialization>() == CentroidInitialization::CustomCentroids) {

        if (e->type() == QEvent::MouseMove) {
            QMouseEvent * me = (QMouseEvent*)e;
            auto viewport = m_graph->viewportRect();
            if (m_dragging) {
                m_markers[m_selectedCentroid]->setValue(m_graph->horizontalAxis()->value(me->pos().x() - viewport.left()) + m_dragOffset.x(), m_graph->verticalAxis()->value(me->pos().y() - viewport.top()) + m_dragOffset.y());
                m_graph->update();
            } else {
                m_selectedCentroid = -1;
                for (int i = 0; i < m_markers.size(); ++i) {
                    Point p(viewport.left() + m_graph->horizontalAxis()->pixel(m_markers[i]->x()), viewport.top() + m_graph->verticalAxis()->pixel(m_markers[i]->y()));
                    double D = p.distanceTo(Point(me->pos().x(), me->pos().y()));
                    if (D < m_markers[i]->size() / 2) {
                        m_selectedCentroid = i;
                        break;
                    }
                }
                m_graph->setCursor((m_selectedCentroid >= 0) ? Qt::PointingHandCursor : Qt::ArrowCursor);
            }
        }
        else if (e->type() == QEvent::MouseButtonPress && m_selectedCentroid >= 0) {
            QMouseEvent * me = (QMouseEvent*)e;
            m_graph->setCursor(Qt::ClosedHandCursor);
            auto viewport = m_graph->viewportRect();
            Point p(m_graph->horizontalAxis()->value(me->position().x() - viewport.left()), m_graph->verticalAxis()->value(me->position().y() - viewport.top()));
            m_dragOffset = Point(m_markers[m_selectedCentroid]->x(), m_markers[m_selectedCentroid]->y()) - p;
            m_dragging = true;
        }
        else if (e->type() == QEvent::MouseButtonRelease && m_dragging) {
            m_graph->setCursor(Qt::PointingHandCursor);
            m_dragging = false;
        }
    }

    return QObject::eventFilter(obj, e);
}

void EMClusterMethodWidget::numClustersChanged()
{
    auto method = m_centroidInit->itemData(m_centroidInit->currentIndex()).value<CentroidInitialization>();
    if (method == CentroidInitialization::CustomCentroids) {

        if (m_data->design()->isValid()) {

            initializeCentroidMarkers();

        } else {

            if (m_numClusters->value() > m_markers.size()) {

                // add colour based, then random
                // so, this is temporary
                // might be better to use furthest distance method
                // todo

                auto random = m_data->randomSelectedPoints(m_numClusters->value() - m_markers.size());
                for (const auto & pt : random) {
                    m_markers.push_back(new Plot::RingMarker(m_graph->horizontalAxis(), m_graph->verticalAxis(), pt.x(), pt.y()));
                    m_graph->addDynamicPrimitive(m_markers.back());
                }

            } else if (m_numClusters->value() < m_markers.size()) {
                for (int i = m_numClusters->value(); i < m_markers.size(); ++i) {
                    m_graph->removeDynamicPrimitive(m_markers[i]);
                    delete m_markers[i];
                }
                m_markers.resize(m_numClusters->value());
            }
        }

        m_graph->update();
    }
}

void EMClusterMethodWidget::initMethodChanged()
{
    auto method = m_centroidInit->itemData(m_centroidInit->currentIndex()).value<CentroidInitialization>();
    if (method == CentroidInitialization::CustomCentroids) {
        if (m_replicates->isEnabled()) {
            m_backupNumReplicates = m_replicates->value();
            m_backupSampleSize = m_sampleSize->value();
        }
        m_replicates->setEnabled(false);
        m_replicates->setValue(1);
        m_sampleSize->setEnabled(false);
        m_sampleSize->setValue(0);
        m_initBoxLayout->setRowVisible(m_reinitialize, true);
    } else if (method == CentroidInitialization::CurrentColors) {
        if (m_replicates->isEnabled()) {
            m_backupNumReplicates = m_replicates->value();
            m_backupSampleSize = m_sampleSize->value();
        }
        m_replicates->setEnabled(false);
        m_replicates->setValue(1);
        m_sampleSize->setEnabled(false);
        m_sampleSize->setValue(0);
        m_initBoxLayout->setRowVisible(m_reinitialize, false);
    }
    else if (!m_replicates->isEnabled()) {
        m_replicates->setValue(m_backupNumReplicates);
        m_sampleSize->setValue(m_backupSampleSize);
        m_replicates->setEnabled((method != CentroidInitialization::CurrentColors));
        m_sampleSize->setEnabled((method != CentroidInitialization::CurrentColors));
        m_initBoxLayout->setRowVisible(m_reinitialize, false);
    }

    if (method == CentroidInitialization::CustomCentroids) {
        initializeCentroidMarkers();
    } else if (m_markers.size() > 0) {
        for (auto * marker : m_markers) {
            m_graph->removeDynamicPrimitive(marker);
            delete marker;
        }
        m_markers.clear();
        m_graph->update();
    }
}

void EMClusterMethodWidget::initializeCentroidMarkers()
{
    for (auto * marker : m_markers) {
        m_graph->removeDynamicPrimitive(marker);
        delete marker;
    }
    m_markers.clear();
    std::vector<Point> centroids;
    if (m_data->design()->isValid())
        centroids = CentroidsFromDesign::generate(m_data, m_numClusters->value());
    else
        centroids = CentroidsFromCurrentColors::generate(m_data, m_numClusters->value());
    for (int i = 0; i < m_numClusters->value(); ++i) {
        m_markers.push_back(new Plot::RingMarker(m_graph->horizontalAxis(), m_graph->verticalAxis(), centroids[i].x(), centroids[i].y()));
        m_graph->addDynamicPrimitive(m_markers.back());
    }
    m_graph->update();
}

void EMClusterMethodWidget::run()
{
    auto method = m_centroidInit->itemData(m_centroidInit->currentIndex()).value<CentroidInitialization>();

    if (!workerThread) {
        setEnabled(false);
        emit beginClustering();
        workerThread = new QThread;
        EMWorker * worker;
        if (method != CentroidInitialization::CustomCentroids) {
            auto container = getClusteringContainer();
            worker = new EMWorker(container);
        } else {
            std::vector<Point> centroids;
            for (auto * marker : m_markers)
                centroids.push_back({marker->x(), marker->y()});
            auto container = getClusteringContainer(centroids);
            worker = new EMWorker(container);
        }
        worker->moveToThread(workerThread);
        connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &EMClusterMethodWidget::startThread, worker, &EMWorker::go);
        connect(worker, &EMWorker::finished, this, &EMClusterMethodWidget::threadFinished);
        connect(worker, &EMWorker::updateProgress, this, &EMClusterMethodWidget::updateProgress);
        connect(this, &EMClusterMethodWidget::cancelThread, worker, &EMWorker::cancel);
        workerThread->start();
        emit startThread();
    }
}

void EMClusterMethodWidget::cancel()
{
    emit cancelThread();
    stopThread();
}

void EMClusterMethodWidget::threadFinished()
{
    stopThread();
}

void EMClusterMethodWidget::stopThread()
{
    if (workerThread) {
        workerThread->quit();
        workerThread->wait();
        workerThread->deleteLater();
        workerThread = nullptr;
        emit endClustering();
        setEnabled(true);
    }
}
