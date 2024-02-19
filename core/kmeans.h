#ifndef FUZZYDROPLETS_CORE_KMEANS_H
#define FUZZYDROPLETS_CORE_KMEANS_H

#include "emclustering.h"

class KMeans : public EMClustering<Point>
{
public:

    KMeans(Data * data, double m_fuzzy, int numClusters, int numReplicates, int maxIters, CentroidInitialization method, int sampleSize, const std::vector<Point> & customInitCentroids = std::vector<Point>());

    void expectation();
    long double maximization();
    void finalizeReplicates();

    Point getDistributionData(const Point & centroid) {return centroid;}

private:

    long double m_bestDistance {std::numeric_limits<long double>::max()};
    double m_fuzzy {1.0};
};

#endif // FUZZYDROPLETS_CORE_KMEANS_H
