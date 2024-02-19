#ifndef FUZZY_DROPLETS_APPROXIMATELY_HPP
#define FUZZY_DROPLETS_APPROXIMATELY_HPP

#include <concepts>
#include <limits>
#include <algorithm>

// approximate comparisons for floating point numbers

namespace approximately
{
    template <std::floating_point T>
    inline bool equalsZero(T x)
    {
        return std::abs(x) <= std::numeric_limits<T>::epsilon() * std::max(std::abs(x), 1.0);
    }

    template <std::floating_point T>
    inline bool equals(T x, T y)
    {
        return (std::abs(x - y) <= std::numeric_limits<T>::epsilon() * std::max(1.0, std::max(std::abs(x), std::abs(y))));
    }

    template <std::floating_point T>
    inline bool lessThan(T x, T y)
    {
        return x < y + std::numeric_limits<T>::epsilon() * std::max(1.0, std::abs(y));
    }

    template <std::floating_point T>
    inline bool greaterThan(T x, T y)
    {
        return x > y - std::numeric_limits<T>::epsilon() * std::max(1.0, std::abs(y));
    }

    template <std::floating_point T>
    inline bool inRange(T test, T x0, T x1)
    {
        return (lessThan(test, x0) && greaterThan(test, x1)) || (lessThan(test, x1) && greaterThan(test, x0));
    }
}

#endif // FUZZY_DROPLETS_APPROXIMATELY_HPP
