#include "gmm.h"
#include "fuzzycolor.h"
#include <execution>

#ifdef Q_OS_MACOS
#include <QtConcurrent>
#endif

Gmm::Gmm(Data * d, bool clusterOutliers, bool fixedMeans, bool sharedScale, bool sharedRho, DefuzzificationPolicy policy, int numClusters, int numReplicates, int maxIters, CentroidInitialization method, int sampleSize, const std::vector<Point> & customInitCentroids)
    : EMClustering(d, numClusters, numReplicates, maxIters, sampleSize, method, customInitCentroids),
    m_policy(policy),
    m_clusterOutliers(clusterOutliers),
    m_fixedMeans(fixedMeans),
    m_sharedScale(sharedScale),
    m_sharedRho(sharedRho)
{
    m_alpha = std::vector<double>(numClusters + 1, 0);
    if (m_clusterOutliers)
        std::fill(m_alpha.begin(), m_alpha.end(), 1.0 / (numClusters + 1));
    else
        std::fill(m_alpha.begin() + 1, m_alpha.end(), 1.0 / numClusters);
    m_uni = 1.0 / (data()->bounds().width() * data()->bounds().height());
    setEps(1); // todo, check
}

void Gmm::expectation()
{
#ifndef Q_OS_MACOS
    std::for_each(std::execution::par, pointIota().begin(), pointIota().end(), [&](size_t i){
#else
    QtConcurrent::blockingMap(pointIota().begin(), pointIota().end(), [&](const size_t & i){
#endif
        FuzzyColor color((int)data()->colorComponentCount());
        double denom = 0;
        for (int k = 1; k <= numClusters(); ++k) {
            denom += m_alpha[k] * distribution(k-1).pdf(data()->point(i).x(), data()->point(i).y());
        }
        if (m_clusterOutliers)
            denom += m_alpha[0] * m_uni;
        if (denom == 0) denom = std::numeric_limits<double>::min();

        for (int k = 1; k <= numClusters(); ++k)
            color.setWeight(k, (m_alpha[k] * distribution(k-1).pdf(data()->point(i).x(), data()->point(i).y()))/denom);
        if (m_clusterOutliers)
            color.setWeight(0, (m_alpha[0] * m_uni) / denom);
        data()->storeColor(i, color);
    });
}



long double Gmm::maximization()
{
    // update alpha
    std::fill(m_alpha.begin(), m_alpha.end(), 0);
    for (auto i : pointIota()) {
        for (int k =1 ; k <= numClusters(); ++k)
            m_alpha[k] += data()->fuzzyColor(i).weight(k);
        if (m_clusterOutliers)
            m_alpha[0] += data()->fuzzyColor(i).weight(0);
    }
    for (auto & a : m_alpha)
        a /= pointIota().size();

    // update means

    if (!m_fixedMeans) {
        auto mu = data()->centroidsByFuzzyColor(Data::Selected);
        for (int k = 1; k <= numClusters(); ++k)
            distribution(k-1).setMean(mu[k].x(), mu[k].y());
    }

    // update scale

    if (m_sharedScale) {
        WeightedArithmeticMean<double> sx;
        WeightedArithmeticMean<double> sy;
        for (int k = 0; k < numClusters(); ++k) {
            for (auto i : pointIota()) {
                sx.add(pow(distribution(k).meanX() - data()->point(i).x(), 2), data()->fuzzyColor(i).weight(k+1));
                sy.add(pow(distribution(k).meanY() - data()->point(i).y(), 2), data()->fuzzyColor(i).weight(k+1));
            }
        }
        for (int k = 0; k < numClusters(); ++k)
            distribution(k).setStdDev(sqrt(sx.mean()), sqrt(sy.mean()));
    } else {
        for (int k=0; k < numClusters(); ++k) {
            WeightedArithmeticMean<double> sxMean;
            WeightedArithmeticMean<double> syMean;
            WeightedArithmeticMean<double> rMean;
            for (auto i : pointIota()) {
                sxMean.add(pow(distribution(k).meanX() - data()->point(i).x(), 2), data()->fuzzyColor(i).weight(k+1));
                syMean.add(pow(distribution(k).meanY() - data()->point(i).y(), 2), data()->fuzzyColor(i).weight(k+1));
            }
            distribution(k).setStdDev(sqrt(sxMean.mean()), sqrt(syMean.mean()));
            distribution(k).setRho(rMean.mean()/(distribution(k).stdDevX() * distribution(k).stdDevY()));
        }
    }

    // update rho

    if (m_sharedRho) {
        ArithmeticMean<double> rho;
        for (auto i : pointIota()) {
            for (int k = 0; k < numClusters(); ++k)
                rho.add((data()->point(i).x() - distribution(k).meanX()) * (data()->point(i).y() - distribution(k).meanY()) * data()->fuzzyColor(i).weight(k+1));
        }
        for (int k= 0; k < numClusters(); ++k)
            distribution(k).setRho(rho.mean() / (distribution(k).stdDevX() * distribution(k).stdDevY()));
    } else {
        for (int k = 0; k < numClusters(); ++k) {
            ArithmeticMean<double> rMean;
            for (auto i : pointIota())
                rMean.add((data()->point(i).x() - distribution(k).meanX()) * (data()->point(i).y() - distribution(k).meanY()) * data()->fuzzyColor(i).weight(k+1));
            distribution(k).setRho(rMean.mean()/(distribution(k).stdDevX() * distribution(k).stdDevY()));
        }
    }

    // calculate score

    long double L = 0;
    for (auto i : pointIota()) {
        long double val = 0;
        for (int k = 0; k < numClusters(); ++k)
            val += data()->fuzzyColor(i).weight(k+1) * distribution(k).pdf(data()->point(i).x(), data()->point(i).y());
        if (m_clusterOutliers)
            val += data()->fuzzyColor(i).weight(0) * m_uni;
        L -= log(val);
    }

    return L;
}

void Gmm::finalizeReplicates()
{
//    for (auto i : pointIota()) {
//        FuzzyColor rFuzz(data()->colorComponentCount());
//        auto fuzz = data()->fuzzyColor(i);
//        if (fuzz.dominantComponent() < numClusters()) {
//            for (int k = 0; k <= numClusters(); ++k)
//                rFuzz.setWeight(k, fuzz.weight(k));
//            rFuzz.normalize();
//        }
//        data()->setColor(i, rFuzz);
//    }
    if (m_policy == DeterministicallyDefuzzify)
        data()->deterministicDefuzzifySelection();
    else if (m_policy == RandomlyDefuzzify)
        data()->randomlyDefuzzifySelection();
}

