#ifndef FUZZYDROPLETS_GUI_CLUSTERING_EMWORKER_H
#define FUZZYDROPLETS_GUI_CLUSTERING_EMWORKER_H

#include <QObject>
#include "../core/emclustering.h"

class Data;

struct EMClusteringContainerBase
{
    virtual ~EMClusteringContainerBase() {}
    virtual int numReplicates() = 0;
    virtual void finish() = 0;
    virtual void storeBestDists() = 0;
    virtual double performReplicate() = 0;
};

template <typename DistributionType>
struct EMClusteringContainer : public EMClusteringContainerBase
{
    std::vector<DistributionType> bestDists;
    EMClustering<DistributionType> * clustering;

    EMClusteringContainer(EMClustering<DistributionType> * clusterer) : clustering(clusterer) {}
    ~EMClusteringContainer() {delete clustering;}

    int numReplicates()
    {
        return clustering->numReplicates();
    }

    void finish()
    {
        clustering->setDistributions(bestDists);
        clustering->expectation();
        clustering->finalizeReplicates();

    }

    void storeBestDists()
    {
        bestDists = clustering->distributions();
    }

    double performReplicate()
    {
        return clustering->performReplicate();
    }
};

class EMWorker : public QObject
{
    Q_OBJECT

public:

    EMWorker(EMClusteringContainerBase * container);
    ~EMWorker() {/*delete m_clustering;*/} // deleted by EMClusterngContainer?

public slots:

    void go();
    void proceedClustering();
    void cancel();

signals:

    void proceed();
    void updateProgress(int);
    void finished();

private:

    EMClusteringContainerBase * m_clustering;
    size_t m_count {0};
    bool m_cancelled{false};
    int m_percent;
    long double m_bestScore;
    std::vector<Point> m_bestCentroids;
};

#endif // FUZZYDROPLETS_GUI_CLUSTERING_EMWORKER_H
