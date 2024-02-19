#include "kmeans.h"
#include "quadtree.h"
#include "data.h"
#include <random>
#include <execution>

KMeans::KMeans(Data * d, double fuzzy, int numClusters, int numReplicates, int maxIters, CentroidInitialization method, int sampleSize, const std::vector<Point> & customInitCentroids)
    : EMClustering(d, numClusters, numReplicates, maxIters, sampleSize, method, customInitCentroids),
    m_fuzzy(fuzzy)
{
    setEps(1);
}

void KMeans::expectation()
{
    if (m_fuzzy > 1) {
        std::for_each(std::execution::par, pointIota().begin(), pointIota().end(), [&](size_t i){
            FuzzyColor color(data()->colorComponentCount());
            for (size_t j = 0; j < numClusters(); ++j) {
                double denom = 0;
                for (size_t k = 0; k < numClusters(); ++k)
                    denom += pow(distribution(j).distanceTo(data()->point(i)) / distribution(k).distanceTo(data()->point(i)), 2.0/(m_fuzzy-1));
                color.setWeight(j+1, 1.0 / denom);
            }
            data()->storeColor(i, color);
        });
    } else {
//        data()->quadTree()->setKMeansLabels(data()->points(), distributions(), [&](size_t i, size_t color){data()->storeColor(i, color + 1);}, [&](size_t i){return data()->isSelected(i);});
        std::for_each(std::execution::par, pointIota().begin(), pointIota().end(), [&](size_t i) {
            long double bestScore = std::numeric_limits<long double>::max();
            int bestK = 0;
            for (int k = 0; k < numClusters(); ++k) {
                long double score = distributions()[k].distanceTo(data()->point(i));
                if (score < bestScore) {
                    bestScore = score;
                    bestK = k;
                }
            }
            data()->setColor(i, bestK + 1);
        });
    }
}

long double KMeans::maximization()
{
    if (m_fuzzy > 1) {
        auto centres = data()->centroidsByFuzzyColor(Data::Selected);
        for (int i = 0; i < numClusters(); ++i)
            distribution(i) = centres[i+1];
        WeightedArithmeticMean<long double> mean;
        for (auto i : pointIota()) {
            for (int j = 0; j < numClusters(); ++j) {
                mean.add(data()->point(i).distanceTo(distribution(j)), data()->fuzzyColor(i).weight(j+1));
            }
        }
        return mean.mean();
    } else {
        auto centres = data()->centroidsByDominantColor(Data::Selected);
        for (int i = 0; i < numClusters(); ++i)
            distribution(i) = centres[i+1];
        long double dist = 0;
        for (int i = 0; i < numClusters(); ++i) {
            distribution(i) = centres[i+1];
            auto trans = std::ranges::views::transform(pointIota(), [&](size_t index)->long double {
                double test = (data()->fuzzyColor(index).dominantComponent() == i+1) ? distribution(i).squaredDistanceTo(data()->point(index)) : 0;
                return test;
            });
            dist += std::reduce(std::execution::par, trans.begin(), trans.end(), (long double)0);
        }
        return dist;
    }
}

void KMeans::finalizeReplicates()
{
    data()->updateRgbaInSelection();
}
