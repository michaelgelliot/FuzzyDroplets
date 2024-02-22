#include "nearestneighbourswidget.h"
#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QThread>
#include "core/data.h"

#ifdef Q_OS_MACOS
#include <QtConcurrent>
#endif

NearestNeighboursWorker::NearestNeighboursWorker(Data * data)
    : m_data(data)
{
    connect(this, &NearestNeighboursWorker::finishedStep, this, &NearestNeighboursWorker::nextStep, Qt::QueuedConnection);
}

NearestNeighboursWorker::~NearestNeighboursWorker()
{
    delete m_tree;
}

void NearestNeighboursWorker::go()
{
    m_tree = new QuadTree<size_t>(m_sourceIndices, [&](size_t i){return m_data->point(i).x();}, [&](size_t i){return m_data->point(i).y();});
    emit finishedStep();
}

void NearestNeighboursWorker::cancel()
{
    m_cancel = true;
}

void NearestNeighboursWorker::nextStep()
{
    if (!m_cancel) {
        size_t end = std::min(m_iterStart + 1000, (size_t)m_targetIndices.size());
        int count = m_iterStart;

#ifndef Q_OS_MACOS
        std::for_each(std::execution::par, m_targetIndices.begin() + m_iterStart, m_targetIndices.begin() + end, [&](size_t target) {
#else
        QtConcurrent::blockingMap(m_targetIndices.begin() + m_iterStart, m_targetIndices.begin() + end, [&](size_t & target) {
#endif
            auto points = m_tree->kNearestNeighbors(m_sourceIndices, m_k, m_data->point(target).x(), m_data->point(target).y());
            FuzzyColor color(m_data->colorComponentCount());
            for (auto point : points) {
                for (int i = 0; i < m_data->colorComponentCount(); ++i) {
                    if (m_weighted) {
                        color.setWeight(i, color.weight(i) + m_data->fuzzyColor(m_sourceIndices[point.first]).weight(i) * 1.0/(m_data->point(m_sourceIndices[point.first]).squaredDistanceTo(m_data->point(target))));
                    } else {
                        color.setWeight(i, color.weight(i) + m_data->fuzzyColor(m_sourceIndices[point.first]).weight(i));
                    }
                }
            }
            color.normalize();
            m_data->setColor(target, color);
            ++count;
            double PC = 100 * count / m_targetIndices.size();
            if (PC > m_percent) {
                m_percent = PC;
                emit updateProgress(m_percent);
            }
        });
    }
    m_iterStart += 1000;
    if (!m_cancel && m_iterStart < (size_t)m_targetIndices.size())
        emit finishedStep();
    else
        emit finished();
}

NearestNeighboursWidget::NearestNeighboursWidget(Data * data, QWidget * parent)
    : m_data(data)
{
    QFormLayout * form = new QFormLayout;
    form->setContentsMargins(0,0,0,0);
    kSpinBox = new QSpinBox;
    kSpinBox->setRange(10, 500);
    kSpinBox->setValue(50);
    form->addRow("Number of neighbours", kSpinBox);
    m_weightingCheckBox = new QCheckBox;
    m_weightingCheckBox->setChecked(true);
    form->addRow("Weight by d<sup>-2</sup>", m_weightingCheckBox);
    setLayout(form);
}

void NearestNeighboursWidget::run(const QList<size_t> & sourceIndices, const QList<size_t> & targetIndices)
{
    if (!assignmentWorkerThread) {
        emit beginAssignment();
        setEnabled(false);
        assignmentWorkerThread = new QThread;
        NearestNeighboursWorker * worker = new NearestNeighboursWorker(m_data);
        worker->setParams(kSpinBox->value(), m_weightingCheckBox->isChecked(), sourceIndices, targetIndices);
        worker->moveToThread(assignmentWorkerThread);
        connect(this, &NearestNeighboursWidget::startAssignment, worker, &NearestNeighboursWorker::go);
        connect(worker, &NearestNeighboursWorker::finished, this, &NearestNeighboursWidget::assignmentThreadFinished);
        connect(worker, &NearestNeighboursWorker::updateProgress, this, &NearestNeighboursWidget::updateProgress);
        connect(assignmentWorkerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &NearestNeighboursWidget::cancelAssignment, worker, &NearestNeighboursWorker::cancel);
        assignmentWorkerThread->start();
        emit startAssignment();
    }
}

void NearestNeighboursWidget::cancel()
{
    emit cancelAssignment();
    assignmentThreadFinished();
}

void NearestNeighboursWidget::assignmentThreadFinished()
{
    if (assignmentWorkerThread) {
        assignmentWorkerThread->quit();
        assignmentWorkerThread->wait();
        assignmentWorkerThread->deleteLater();
        assignmentWorkerThread = nullptr;
        emit endAssignment();
        setEnabled(true);
    }
}
