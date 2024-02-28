#include "dbscanwidget.h"
#include "gui/dropletgraphwidget.h"
#include <QFormLayout>
#include <QSpinBox>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QMouseEvent>
#include <QThread>
#include <unordered_set>
#include "core/data.h"

DBScanWidget::DBScanWidget(Data * data, DropletGraphWidget * graph, QWidget *parent)
    : ClusterMethodWidget{parent},
    m_data(data),
    m_graph(graph)
{
    QFormLayout * form = new QFormLayout;
    setLayout(form);
    form->setContentsMargins(0,0,0,0);

    m_epsilon = new QSpinBox(this);
    m_epsilon->setRange(20, 5000);
    m_epsilon->setValue(250);
    m_epsilon->setSingleStep(10);
    form->addRow("Epsilon", m_epsilon);

    m_minPts = new QSpinBox(this);
    m_minPts->setRange(5, 1000);
    m_minPts->setSingleStep(5);
    m_minPts->setValue(20);
    form->addRow("Min Points", m_minPts);
}

DBScanWidget::~DBScanWidget()
{
    if (workerThread) {
        workerThread->quit();
        workerThread->wait();
        delete workerThread;
    }
}

void DBScanWidget::run()
{
    if (!workerThread) {
        setEnabled(false);
        emit beginClustering();
        workerThread = new QThread;
        DBScanWorker * worker = new DBScanWorker(m_data, m_epsilon->value(), m_minPts->value());
        worker->moveToThread(workerThread);
        connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(worker, &DBScanWorker::updateProgress, this, &ClusterMethodWidget::updateProgress);
        connect(this, &DBScanWidget::startThread, worker, &DBScanWorker::go);
        connect(this, &DBScanWidget::cancelThread, worker, &DBScanWorker::cancel);
        connect(worker, &DBScanWorker::finished, this, &DBScanWidget::threadFinished);
        workerThread->start();
        emit startThread();
    }
}

void DBScanWidget::cancel()
{
    threadFinished();
}

void DBScanWidget::threadFinished()
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

DBScanWorker::DBScanWorker(Data * data, double epsilon, int minPts, QObject * parent)
    : QObject(parent),
    m_epsilon(epsilon),
    m_minPts(minPts),
    m_data(data)
{
    m_epsilon2 = pow(m_epsilon, 2);
    connect(this, &DBScanWorker::keepGoing, this, &DBScanWorker::proceed, Qt::QueuedConnection);
}

void DBScanWorker::go()
{
    emit updateProgress(0);
    m_percent = 0;
     m_completedPointCount = 0;
     m_curPos = 0;
     m_assignment = std::vector<size_t>(m_data->pointCount(), std::numeric_limits<size_t>::max()); // for some reason if i use DBScanWorker::Undefined here, it doesn't compile under mac debug
     m_selectedPointCount = m_data->selectedPointCount();
    emit keepGoing();
}

void DBScanWorker::proceed()
{
    if (m_cancelled || m_curPos == m_data->pointCount()) {
        emit updateProgress(100);
        std::vector<std::pair<size_t, size_t>> clusterSizes;
        for (size_t i = 1; i <= m_currentCluster; ++i) {
            size_t count = 0;
            for (size_t k = 0; k < m_assignment.size(); ++k)
                count += (m_assignment[k] == i);
            clusterSizes.push_back({i, count});
        }
        std::sort(clusterSizes.begin(), clusterSizes.end(), [](const auto & left, const auto & right){return left.second > right.second;});

        for (size_t i = 0; i < m_data->pointCount(); ++i) {
            if (m_data->isSelected(i))
                m_data->setColor(i, 0);
        }
        size_t colorId = 1;
        for (size_t i = 0; i < std::min(clusterSizes.size(), m_data->colorComponentCount() - 1); ++i) {
            for (size_t k = 0; k  < m_assignment.size(); ++k) {
                if (m_assignment[k] == clusterSizes[i].first)
                    m_data->setColor(k, colorId);
            }
            ++colorId;
        }

        emit finished();
        return;
    }

    if (m_assignment[m_curPos] != Undefined || !m_data->isSelected(m_curPos)) {
        ++m_curPos;
        emit keepGoing();
        return;
    }

    increaseCompletedPointCount();

    auto value = m_data->point(m_curPos);
    OrthogonalRectangle rect(value + Point(-m_epsilon, -m_epsilon), m_epsilon * 2, m_epsilon * 2);
    auto neighbours = m_data->rectangleSearchSelection(rect, [&](size_t i) {
        return pow(m_data->point(i).x() - value.x(),2) + pow((m_data->point(i).y() - value.y()),2) < m_epsilon2;
    });
    if (neighbours.size() < m_minPts) {
        m_assignment[m_curPos] = 0;
        ++m_curPos;
        emit keepGoing();
        return;
    }

    std::unordered_set<size_t> seeds(neighbours.begin(), neighbours.end());
    ++ m_currentCluster;
    m_assignment[m_curPos] = m_currentCluster;
    while (!seeds.empty()) {
        auto neighbour = *(seeds.begin());
        seeds.erase(seeds.begin());
        if (neighbour == m_curPos) continue;
        if (m_assignment[neighbour] == 0)
            m_assignment[neighbour] = m_currentCluster;
        if (m_assignment[neighbour] == Undefined) {
            increaseCompletedPointCount();
            m_assignment[neighbour] = m_currentCluster;
            value = m_data->point(neighbour);
            rect = OrthogonalRectangle(value + Point(-m_epsilon, -m_epsilon), m_epsilon * 2, m_epsilon * 2);
            neighbours = m_data->rectangleSearchSelection(rect, [&](size_t i) {
                return pow(m_data->point(i).x() - value.x(),2) + pow((m_data->point(i).y() - value.y()),2) < m_epsilon2;
            });
            if (neighbours.size() > m_minPts) {
#ifndef Q_OS_WIN
                seeds.reserve(seeds.size() + neighbours.size());
                for (auto & n : neighbours)
                    seeds.insert(n);
#else
                seeds.insert_range(neighbours);
#endif
            }
        }
    }

    ++m_curPos;
    emit keepGoing();
}

void DBScanWorker::increaseCompletedPointCount()
{
    ++m_completedPointCount;
    int oldPc = m_percent;
    m_percent = 100 * m_completedPointCount / m_selectedPointCount;
    if (m_percent != oldPc) {
        emit updateProgress(m_percent);
    }
}

void DBScanWorker::cancel()
{
    m_cancelled = true;
}
