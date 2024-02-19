#ifndef FUZZYDROPLETS_CORE_GMM_H
#define FUZZYDROPLETS_CORE_GMM_H

#include "binormal.h"
#include "emclustering.h"

// Gaussian mixture model

class Gmm : public EMClustering<BinormalDistribution>
{
public:

    Gmm(Data * data, bool clusterOutliers, bool fixedMeans, bool sharedScale, bool sharedRho, EMClustering<BinormalDistribution>::DefuzzificationPolicy policy, int numClusters, int numReplicates, int maxIters, CentroidInitialization method, int sampleSize, const std::vector<Point> & customInitCentroids = std::vector<Point>());

    void expectation();
    long double maximization();
    void finalizeReplicates();
    BinormalDistribution getDistributionData(const Point & centroid) {return BinormalDistribution(centroid.x(), centroid.y(), 1000, 1000, 0);}

private:

    DefuzzificationPolicy m_policy {Fuzzy};
    std::vector<double> m_alpha; // prior probability of each dist
    bool m_clusterOutliers {false};
    bool m_fixedMeans {false};
    bool m_sharedScale {false};
    bool m_sharedRho {false};
    long double m_uni;

};

#endif // FUZZYDROPLETS_CORE_GMM_H
