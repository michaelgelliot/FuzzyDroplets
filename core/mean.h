#ifndef FUZZY_DROPLETS_MEAN_HPP
#define FUZZY_DROPLETS_MEAN_HPP

#include <array>
#include <cassert>
#include "geometry.h"

template <typename T>
class ArithmeticMean;

template <typename T>
class WeightedArithmeticMean;

template <typename T>
class GeometricMean;

template <typename T>
class HarmonicMean;

// unidimensional values

template <>
class ArithmeticMean<double>
{
public:

    void add(double t)
    {
        m_mean += (t - m_mean) / ++m_count;
    }

    double mean() const
    {
        return m_mean;
    }

    size_t count() const
    {
        return m_count;
    }

private:

    size_t m_count {0};
    double m_mean {0};
};

template <std::floating_point T>
class WeightedArithmeticMean<T>
{
public:

    void add(T t, T weight)
    {
        m_count += weight;
        if (m_count > 0)
            m_mean += ((t - m_mean)*weight) / m_count;
    }

    T mean() const
    {
        return m_mean;
    }

    T count() const
    {
        return m_count;
    }

private:

    T m_count {0};
    T m_mean {0};
};

template <>
class GeometricMean<double>
{
public:

    void add(double t)
    {
        m_mean += log(t);
        ++m_count;
    }

    double mean() const
    {
        return exp(m_mean / m_count);
    }

    size_t count() const
    {
        return m_count;
    }

private:

    size_t m_count {0};
    double m_mean {0};
};

template <>
class HarmonicMean<double>
{
public:

    void add(double t)
    {
        m_mean += 1.0/t;
        ++m_count;
    }

    double mean() const
    {
        return m_count / m_mean;
    }

    size_t count() const
    {
        return m_count;
    }

private:

    size_t m_count {0};
    double m_mean {0};
};

// multidimensional values stored in arrays, vectors or similar

template <size_t N>
class ArithmeticMean<std::array<double, N>>
{
public:

    void add(std::array<double, N> t)
    {
        ++m_count;
        for (size_t i = 0; i < m_mean.size(); ++i)
            m_mean[i] += (t[i] - m_mean[i]) / m_count;
    }

    std::array<double, N> mean() const
    {
        return m_mean;
    }

    size_t count() const
    {
        return m_count;
    }

private:

    size_t m_count {0};
    std::array<double, N> m_mean {0};
};

template<>
class ArithmeticMean<std::vector<double>>
{
public:

    ArithmeticMean(size_t n)
    {
        m_mean = std::vector<double>(n, 0);
    }

    void add(const std::vector<double> & t)
    {
        assert(t.size() == m_mean.size());
        ++m_count;
        for (size_t i = 0; i < m_mean.size(); ++i)
            m_mean[i] += (t[i] - m_mean[i]) / m_count;
    }

    std::vector<double> mean() const
    {
        return m_mean;
    }

    size_t count() const
    {
        return m_count;
    }

private:

    size_t m_count {0};
    std::vector<double> m_mean;
};

template <>
class ArithmeticMean<Point>
{
public:

    void add(Point t)
    {
        ++m_count;
        m_mean += (t - m_mean) / m_count;
    }

    Point mean() const
    {
        return m_mean;
    }

    size_t count() const
    {
        return m_count;
    }

private:

    size_t m_count {0};
    Point m_mean {0,0};
};

template <>
class WeightedArithmeticMean<Point>
{
public:

    void add(Point t, double weight)
    {
        m_count += weight;
        if (m_count > 0)
            m_mean += ((t - m_mean) * weight) / m_count;
    }

    Point mean() const
    {
        return m_mean;
    }

    double count() const
    {
        return m_count;
    }

private:

    double m_count {0};
    Point m_mean {0,0};
};

template <size_t N>
class GeometricMean<std::array<double, N>>
{
public:

    void add(std::array<double, N> t)
    {
        for (size_t i = 0; i < m_mean.size(); ++i)
            m_mean[i] += log(t[i]);
        ++m_count;
    }

    std::array<double, N> mean() const
    {
        auto z = m_mean;
        for (size_t i = 0; i < m_mean.size(); ++i)
            z[i] = exp(z[i] / m_count);
        return z;
    }

    size_t count() const
    {
        return m_count;
    }

private:

    size_t m_count {0};
    std::array<double, N> m_mean {0};
};

template <size_t N>
class HarmonicMean<std::array<double, N>>
{
public:

    void add(std::array<double, N> t)
    {
        for (size_t i = 0; i < m_mean.size(); ++i)
            m_mean[i] += 1.0 / t[i];
        ++m_count;
    }

    std::array<double, N> mean() const
    {
        auto z = m_mean;
        for (size_t i = 0; i < N; ++i)
            z[i] = m_count / z[i];
        return z;
    }

    size_t count() const
    {
        return m_count;
    }

private:

    size_t m_count {0};
    std::array<double, N> m_mean {0};
};

#endif // FUZZY_DROPLETS_MEAN_HPP
