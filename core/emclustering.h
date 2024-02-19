#ifndef FUZZYDROPLETS_CORE_EMCLUSTERING_H
#define FUZZYDROPLETS_CORE_EMCLUSTERING_H

#include "data.h"
#include "centroids.h"
#include "geometry.h"
#include <vector>

template <typename DistributionData>
class EMClustering
{
public:

    enum DefuzzificationPolicy {
        Fuzzy,
        DeterministicallyDefuzzify,
        RandomlyDefuzzify
    };


    EMClustering(Data * data, int numClusters, int numReplicates, int maxIters, int sampleSize, CentroidInitialization method, const std::vector<Point> & customInitCentroids )
        : m_data(data),
          m_numClusters(numClusters),
          m_numReplicates(numReplicates),
          m_maxIters(maxIters),
          m_method(method),
          m_customInitCentroids(customInitCentroids)
    {
        if (m_data->colorComponentCount() < numClusters + 1)
            m_data->setColorComponentCount(numClusters + 1);

        if (method == CentroidInitialization::KMeansPP || method == CentroidInitialization::FarthestPoint)
            m_initSample = data->randomSelectedPoints(std::max(sampleSize, numClusters));

        m_pointIota = std::vector<int>();
        m_pointIota.reserve(data->selectedPointCount());
        for (int i = 0; i < data->pointCount(); ++i)
            if (data->isSelected(i)) m_pointIota.push_back(i);

        m_dists.resize(numClusters);
    }

    void setEps(double eps) {m_eps = eps;}
    double eps() const {return m_eps;}

    virtual void expectation() = 0;
    virtual long double maximization() = 0;
    virtual void finalizeReplicates() {}
    virtual DistributionData getDistributionData(const Point & centroid) = 0;

    Data * data() {return m_data;}
    DistributionData & distribution(size_t i) {return m_dists[i];}
    void setDistributions(const std::vector<DistributionData> & distributions) {m_dists = distributions;}
    const std::vector<DistributionData> & distributions() const {return m_dists;}

    int numClusters() const {return m_numClusters;}
    int numReplicates() const {return m_numReplicates;}
    int maxIters() const {return m_maxIters;}

    const std::vector<int> & pointIota() const {return m_pointIota;}

    long double performReplicate()
    {
        std::vector<Point> centroids;
        switch (m_method) {
        case CentroidInitialization::CurrentColors :   centroids = CentroidsFromCurrentColors::generate(data(), (int)m_dists.size()); break;
        case CentroidInitialization::RandomCentroids : centroids = RandomCentroids::generate(data(), (int)m_dists.size()); break;
        case CentroidInitialization::RandomMedoids :   centroids = RandomMedoids::generate(data(), (int)m_dists.size()); break;
        case CentroidInitialization::FarthestPoint :   centroids = FarthestDistance::generate(data(), m_initSample, (int)m_dists.size()); break;
        case CentroidInitialization::KMeansPP :        centroids = KMeansPP::generate(data(), m_initSample, (int)m_dists.size()); break;
        case CentroidInitialization::CustomCentroids : centroids = m_customInitCentroids; break;
        }

        for (int i = 0; i < m_dists.size(); ++i) {
            m_dists[i] = getDistributionData(centroids[i]);
        }

        expectation();
        long double score = maximization();
        auto improvement = score;
        int count = 0;
        while (improvement > m_eps) {
            ++count;
            expectation();
            auto newScore = maximization();
            improvement = score - newScore;
            score = newScore;
            if (count > m_maxIters) {
                qDebug("hit max iters");
                break;
            }
        }
        return score;
    }

    void performReplicates()
    {
        double bestScore = std::numeric_limits<double>::max();
        std::vector<DistributionData> bestDists;
        for (int i = 0; i < numReplicates(); ++i) {
            double val = performReplicate();
            if (val < bestScore) {
                bestScore = val;
                bestDists = m_dists;
            }
        }
        m_dists = bestDists;
        expectation();
        finalizeReplicates();
    }

private:

    Data * m_data;
    int m_numClusters;
    int m_numReplicates;
    int m_maxIters;
    std::vector<int> m_pointIota;
    std::vector<Point> m_initSample;
    CentroidInitialization m_method;
    std::vector<Point> m_customInitCentroids;
    std::vector<DistributionData> m_dists;
    double m_eps{1};
};

#endif // FUZZYDROPLETS_CORE_EMCLUSTERING_H
