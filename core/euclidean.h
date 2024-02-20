#ifndef FUZZYDROPLETS_CORE_EUCLIDEAN_HPP
#define FUZZYDROPLETS_CORE_EUCLIDEAN_HPP

#include <ranges>
#include <numeric>
#include "geometry.h"

template <typename Tag>
struct DistanceMetric;

struct Euclidean;
struct SquaredEuclidean;

template <>
struct DistanceMetric<SquaredEuclidean>
{
    template <std::ranges::range T>
    inline double operator()(const T & a, const T & b) const
    {
        auto view = std::views::zip(a, b);
        return std::accumulate(view.begin(), view.end(), double(0.0), [](double d, const auto & elem){return d + pow(std::get<0>(elem) - std::get<1>(elem), 2);});
    }

    inline double operator()(const Point & a, const Point & b) const
    {
        return pow(a.x() - b.x(), 2) + pow(a.y() - b.y(), 2);
    }
};

template <>
struct DistanceMetric<Euclidean>
{
    template <typename T>
    inline double operator()(const T & a, const T & b) const
    {
        return sqrt(dm(a,b));
    }

    DistanceMetric<SquaredEuclidean> dm;
};

#endif // FUZZYDROPLETS_CORE_EUCLIDEAN_HPP