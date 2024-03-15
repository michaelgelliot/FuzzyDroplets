#include "gmmassignmentwidget.h"
#include <QFormLayout>
#include <QThread>
#include <QCheckBox>
#include "../core/data.h"
#include "../core/mean.h"

#ifdef Q_OS_MACOS
#include <QtConcurrent>
#endif

GmmAssignmentWorker::GmmAssignmentWorker(Data * data)
    : m_data(data)
{
    connect(this, &GmmAssignmentWorker::finishedStep, this, &GmmAssignmentWorker::nextStep, Qt::QueuedConnection);
}

GmmAssignmentWorker::~GmmAssignmentWorker()
{
}

void GmmAssignmentWorker::go()
{
    m_alpha = std::vector<double>(m_data->colorComponentCount(), 0);
    for (auto i : m_sourceIndices) {
        for (int k = 0 ; k < m_data->colorComponentCount(); ++k)
            m_alpha[k] += m_data->fuzzyColor(i).weight(k);
    }
    for (auto & a : m_alpha) {
        a /= m_sourceIndices.size();
    }

    m_distributions.resize(m_data->colorComponentCount());
    QList<WeightedArithmeticMean<Point>> means(m_data->colorComponentCount());
    for (auto source : m_sourceIndices) {
        for (int k = 0; k < m_data->colorComponentCount(); ++k) {
            means[k].add(m_data->point(source), m_data->fuzzyColor(source).weight(k));
        }
    }
    for (int k = 0; k < m_data->colorComponentCount(); ++k) {
        m_distributions[k].setMean(means[k].mean().x(), means[k].mean().y());
    }

    if (m_sharedScale) {
        WeightedArithmeticMean<double> sx;
        WeightedArithmeticMean<double> sy;
        for (int k = 0; k < m_data->colorComponentCount(); ++k) {
            for (auto i : m_sourceIndices) {
                sx.add(pow(m_distributions[k].meanX() - m_data->point(i).x(), 2), m_data->fuzzyColor(i).weight(k));
                sy.add(pow(m_distributions[k].meanY() - m_data->point(i).y(), 2), m_data->fuzzyColor(i).weight(k));
            }
        }
        for (int k = 0; k < m_data->colorComponentCount(); ++k)
            m_distributions[k].setStdDev(sqrt(sx.mean()), sqrt(sy.mean()));
    } else {
        for (int k=0; k < m_data->colorComponentCount(); ++k) {
            WeightedArithmeticMean<double> sxMean;
            WeightedArithmeticMean<double> syMean;
            for (auto i : m_sourceIndices) {
                sxMean.add(pow(m_distributions[k].meanX() - m_data->point(i).x(), 2), m_data->fuzzyColor(i).weight(k));
                syMean.add(pow(m_distributions[k].meanY() - m_data->point(i).y(), 2), m_data->fuzzyColor(i).weight(k));
            }
            m_distributions[k].setStdDev(sqrt(sxMean.mean()), sqrt(syMean.mean()));
        }
    }

    if (m_sharedRho) {
        ArithmeticMean<double> rho;
        for (auto i : m_sourceIndices) {
            for (int k = 0; k < m_distributions.size(); ++k)
                rho.add((m_data->point(i).x() - m_distributions[k].meanX()) * (m_data->point(i).y() - m_distributions[k].meanY()) * m_data->fuzzyColor(i).weight(k));
        }
        for (int k = 0; k < m_distributions.size(); ++k)
            m_distributions[k].setRho(rho.mean() / (m_distributions[k].stdDevX() * m_distributions[k].stdDevY()));
    } else {
        for (int k = 0; k < m_distributions.size(); ++k) {
            ArithmeticMean<double> rMean;
            for (auto i : m_sourceIndices)
                rMean.add((m_data->point(i).x() - m_distributions[k].meanX()) * (m_data->point(i).y() - m_distributions[k].meanY()) * m_data->fuzzyColor(i).weight(k));
            m_distributions[k].setRho(rMean.mean()/(m_distributions[k].stdDevX() * m_distributions[k].stdDevY()));
        }
    }

    emit finishedStep();
}

void GmmAssignmentWorker::cancel()
{
    m_cancel = true;
}

void GmmAssignmentWorker::nextStep()
{
    if (!m_cancel) {
        size_t end = std::min(m_iterStart + 1000, (size_t)m_targetIndices.size());
        auto count = m_iterStart;

#ifndef Q_OS_MACOS
        std::for_each(std::execution::par, m_targetIndices.begin() + m_iterStart, m_targetIndices.begin() + end, [&](size_t target) {
#else
        QtConcurrent::blockingMap(m_targetIndices.begin() + m_iterStart, m_targetIndices.begin() + end, [&](size_t & target) {
#endif
            FuzzyColor color(m_distributions.size());
            for (int i = 0; i < m_distributions.size(); ++i) {
                if (m_alpha[i] > 0) {
                    color.setWeight(i, m_distributions[i].pdf(m_data->point(target).x(), m_data->point(target).y()));
                }
            }
            if (color.totalWeight() > 0) {
                color.normalize();
            } else {
                color.setWeight(0, 1);
            }
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

GmmAssignmentWidget::GmmAssignmentWidget(Data * data, QWidget * parent)
    : m_data(data)
{
    QFormLayout * form = new QFormLayout;
    form->setContentsMargins(0,0,0,0);
    m_sharedScaleCB = new QCheckBox;
    m_sharedRhoCB = new QCheckBox;
    form->addRow("Shared Scale", m_sharedScaleCB);
    form->addRow("Shared Correlation Coefficient", m_sharedRhoCB);
    setLayout(form);
}

void GmmAssignmentWidget::run(const QList<size_t> & sourceIndices, const QList<size_t> & targetIndices)
{
    if (!assignmentWorkerThread) {
        emit beginAssignment();
        setEnabled(false);
        assignmentWorkerThread = new QThread;
        GmmAssignmentWorker * worker = new GmmAssignmentWorker(m_data);
        worker->setParams(m_sharedScaleCB->isChecked(), m_sharedRhoCB->isChecked(), sourceIndices, targetIndices);
        worker->moveToThread(assignmentWorkerThread);
        connect(this, &GmmAssignmentWidget::startAssignment, worker, &GmmAssignmentWorker::go);
        connect(worker, &GmmAssignmentWorker::finished, this, &GmmAssignmentWidget::assignmentThreadFinished);
        connect(worker, &GmmAssignmentWorker::updateProgress, this, &GmmAssignmentWidget::updateProgress);
        connect(assignmentWorkerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &GmmAssignmentWidget::cancelAssignment, worker, &GmmAssignmentWorker::cancel);
        assignmentWorkerThread->start();
        emit startAssignment();
    }
}

void GmmAssignmentWidget::cancel()
{
    emit cancelAssignment();
    assignmentThreadFinished();
}

void GmmAssignmentWidget::assignmentThreadFinished()
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
