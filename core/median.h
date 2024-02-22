#ifndef FUZZY_DROPLETS_MEDIAN_H
#define FUZZY_DROPLETS_MEDIAN_H

#include <algorithm>
#include <execution>
#include <cassert>
#include <QtGlobal>

template <typename RandomIt, typename Compare>
RandomIt median(const RandomIt & first, const RandomIt & last, const Compare & comp)
{
    RandomIt n = first + (last - first - 1) / 2;
    std::nth_element(
#ifndef Q_OS_MACOS
        std::execution::par,
#endif
        first, n, last, comp);
    return n;
}

template <typename RandomIt, typename Compare = std::less<typename RandomIt::value_type>>
RandomIt medianOfMedians(RandomIt first, RandomIt last, const size_t groupSize = 10, Compare comp = Compare())
{
    assert(last > first);
    assert(groupSize > 0);
    RandomIt it = first, swap = first;
    while (it < last) {
        RandomIt end = (size_t(last - it) > groupSize) ? it + groupSize : last;
        std::iter_swap(swap++, median(it, end, comp));
        it = (end == last) ? last : it + groupSize;
    }
    return median(first, swap, comp);
}

//template <typename ExecutionPolicy, typename RandomIt, typename Compare = std::less<typename RandomIt::value_type>>
//RandomIt medianOfMedians(ExecutionPolicy && policy, RandomIt first, RandomIt last, const size_t groupSize = 10, Compare comp = Compare())
//{
//    assert(last > first);
//    assert(groupSize > 0);
//    if (std::is_same<ExecutionPolicy, std::execution::sequenced_policy>::value)
//        return medianOfMedians(first, last, groupSize, comp);
//    auto segments = concurrentContainerSegments((last - first), RandomIt());
//    std::for_each(policy, segments.begin(), segments.end(), [&](auto & segment) {
//        RandomIt it = first + std::get<0>(segment);
//        RandomIt swap = it;
//        auto last = first + std::get<1>(segment) + 1;
//        while (it < last) {
//            RandomIt end = (last - it > groupSize) ? it + groupSize : last;
//            std::iter_swap(swap++, median(it, end, comp));
//            it = (end == last) ? last : it + groupSize;
//        }
//        std::get<2>(segment) = swap;
//    });
//    RandomIt swap = std::get<2>(segments[0]);
//    for (int i = 1; i < segments.size(); ++i)
//        swap = std::swap_ranges(policy, first + std::get<0>(segments[i]), std::get<2>(segments[i]), swap);
//    return median(first, swap, comp);
//}

#endif // FUZZY_DROPLETS_MEDIAN_H
