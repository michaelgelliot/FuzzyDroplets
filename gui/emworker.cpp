#include "emworker.h"

EMWorker::EMWorker(EMClusteringContainerBase * container)
    : m_clustering(container)
{
    connect(this, &EMWorker::proceed, this, &EMWorker::proceedClustering, Qt::QueuedConnection);
}

void EMWorker::go()
{
    emit updateProgress(0);
    m_count = 0;
    m_cancelled = false;
    m_percent = 0;
    m_bestScore = std::numeric_limits<double>::max();
    proceedClustering();
}

void EMWorker::proceedClustering()
{
    if (m_cancelled) {
        emit finished();
        return;
    }

    long double val = m_clustering->performReplicate();
    if (val < m_bestScore) {
        m_bestScore = val;
        m_clustering->storeBestDists();
    }

    int percent = ((double)m_count / m_clustering->numReplicates()) * 100;
    if (m_percent != percent) {
        m_percent = percent;
        emit updateProgress(m_percent);
    }

    ++m_count;

    if (m_count >= m_clustering->numReplicates()) {
        m_clustering->finish();
        emit finished();
    } else {
        emit proceed();
    }
}

void EMWorker::cancel()
{
    m_cancelled = true;
}
