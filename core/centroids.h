#ifndef FUZZYDROPLETS_CORE_CENTROIDS_H
#define FUZZYDROPLETS_CORE_CENTROIDS_H

#include <ranges>
#include <random>
#include "data.h"
#include "mean.h"
#include "euclidean.h"

#include <QtDebug>

enum CentroidInitialization
{
    CurrentColors,
    FarthestPoint,
    KMeansPP,
    RandomCentroids,
    RandomMedoids,
    CustomCentroids
};

struct RandomCentroids
{
    static std::vector<Point> generate(const Data * data, int k)
    {
        std::random_device seed;
        std::mt19937 rng(seed());
        std::vector<Point> result(k);
        std::uniform_real_distribution<double> xDist(data->bounds().left(), data->bounds().right());
        std::uniform_real_distribution<double> yDist(data->bounds().bottom(), data->bounds().top());
        for (int i = 0; i < k; ++i)
            result[i] = {xDist(rng), yDist(rng)};
        return result;
    }
};

struct RandomMedoids
{
    static std::vector<Point> generate(const Data * data, int k)
    {
        return data->randomSelectedPoints(k);
    }
};

struct CentroidsFromDesign
{
    static std::vector<Point> generate(const Data * data, int k)
    {
        std::vector<Point> result = data->design()->clusterCentroids();

        if (result.size() > k) {
            std::vector<double> count(result.size(), 0.0);
            for (size_t i = 0; i < data->points().size(); ++i) {
                if (data->isSelected(i)) {
                    for (size_t k = 0; k < data->colorComponentCount(); ++k) {
                        count[k] += data->fuzzyColor(i).weight(k);
                    }
                }
            }
            auto zip = std::ranges::views::zip(std::ranges::views::iota(0), count);
            std::vector<std::tuple<int,  double>> toSort;
            std::ranges::copy(zip, std::back_inserter(toSort));
            std::ranges::sort(toSort, [&](const auto & left, const auto & right){return std::get<1>(left) > std::get<1>(right);});
            std::transform(toSort.begin(), toSort.end(), result.begin(), [&](const auto & elem){return data->design()->clusterCentroid(std::get<0>(elem));});
            result.resize(k);
        } else if (result.size() < k) {
            result.append_range(RandomMedoids::generate(data, k - (int)result.size()));
        }
        return result;
    }
};

struct CentroidsFromCurrentColors
{
    static std::vector<Point> generate(const Data * data, int k)
    {
        std::vector<WeightedArithmeticMean<Point>> means (data->colorComponentCount());
        for (size_t i = 0; i < data->points().size(); ++i) {
            if (data->isSelected(i)) {
                for (size_t k = 0; k < data->colorComponentCount(); ++k) {
                    means[k].add(data->point(i), data->fuzzyColor(i).weight(k));
                }
            }
        }
        auto zip = std::ranges::views::zip(std::ranges::views::iota(1), means) | std::ranges::views::filter([](const auto & elem){return std::get<1>(elem).count() > 0;});
        std::vector<std::tuple<int,  WeightedArithmeticMean<Point>>> toSort;
        std::ranges::copy(zip, std::back_inserter(toSort));
        std::ranges::sort(toSort, [&](const auto & left, const auto & right){return std::get<1>(left).count() > std::get<1>(right).count();});
        std::vector<Point> result(toSort.size());
        std::transform(toSort.begin(), toSort.end(), result.begin(), [](const auto & elem){return std::get<1>(elem).mean();});
        if (result.size() > k) {
            result.resize(k);
        } else if (result.size() < k) {
            result.append_range(RandomMedoids::generate(data, k - (int)result.size()));
        }

        return result;
    }
};

struct KMeansPP
{
    static std::vector<Point> generate(const Data * data, const std::vector<Point> & filtered, int k)
    {
        std::random_device seed;
        std::mt19937 rng(seed());
        std::vector<Point> result(k);

        std::ranges::sample(filtered, result.begin(), 1, rng);

        std::vector<double> distances(filtered.size());
        auto metric = DistanceMetric<Euclidean>();
        for (size_t i = 1; i < k; ++i) {
            auto zip = std::views::zip(filtered, distances);
            double total = 0;
            std::ranges::for_each(zip, [&](auto elem) {
                auto it = std::min_element(result.begin(), result.begin() + i, [&](const auto & left, const auto & right) {return metric(left, std::get<0>(elem)) < metric(right, std::get<0>(elem));});
                total += metric(*it, std::get<0>(elem));
                std::get<1>(elem) = total;
            });
            std::uniform_real_distribution<> dis(0.0, total);
            auto target = dis(rng);
            auto it = std::ranges::find_if(zip, [&](auto elem){return std::get<1>(elem) > target;});
            result[i] = std::get<0>(*it);
        }
        return result;
    }
};

struct FarthestDistance
{
    static std::vector<Point> generate(const Data * data, const std::vector<Point> & filtered, int k)
    {
        std::random_device seed;
        std::mt19937 rng(seed());
        std::vector<Point> result(k);

        std::ranges::sample(filtered, result.begin(), 1, rng);

        auto metric = DistanceMetric<SquaredEuclidean>();
        for (size_t i = 1; i < k; ++i) {
            double farthest = 0;
            Point val;
            std::ranges::for_each(filtered, [&](const auto & elem) {
                auto it = std::min_element(result.begin(), result.begin() + i, [&](const auto & left, const auto & right) {return metric(left, elem) < metric(right, elem);});
                auto dist = metric(*it, elem);
                if (dist > farthest) {
                    farthest = dist;
                    val = elem;
                }
            });
            result[i] = val;
        }
        double farthest = 0;
        Point val;
        std::ranges::for_each(filtered, [&](const auto & elem) {
            auto it = std::min_element(result.begin() + 1, result.end(), [&](const auto & left, const auto & right) {return metric(left, elem) < metric(right, elem);});
            auto dist = metric(*it, elem);
            if (dist > farthest) {
                farthest = dist;
                val = elem;
            }
        });
        result[0] = val;
        return result;
    }
};

#endif // FUZZYDROPLETS_CORE_CENTROIDS_H
