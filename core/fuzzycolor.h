#ifndef FUZZY_DROPLETS_FUZZYCOLOR_H
#define FUZZY_DROPLETS_FUZZYCOLOR_H

#include <vector>
#include <cassert>
#include <numeric>
#include "color.h"
#include "approximately.h"

class FuzzyColor
{
public:

    FuzzyColor(size_t numComponents = 0)
        : m_weights(numComponents, 0.0)
    {
    }

    FuzzyColor(const std::vector<double> & weights)
    {
        m_weights = weights;
    }

    FuzzyColor(std::vector<double> && weights)
    {
        m_weights = std::move(weights);
    }

#ifndef Q_OS_MACOS
    friend auto operator<=>(const FuzzyColor &, const FuzzyColor &) = default;
#else
    bool operator !=(const FuzzyColor & other) const {return other.m_weights != m_weights;}
#endif

    size_t componentCount() const {return m_weights.size();}

    void setComponentCount(size_t i)
    {
        if (i > m_weights.size()) {
            m_weights.resize(i, 0);
        } else if (i < m_weights.size()) {
            double val = std::accumulate(m_weights.begin() + i, m_weights.end(), 0);
            m_weights[0] += val;
            m_weights.resize(i);
        }
    }

    void setFixedComponent(size_t fixed)
    {
        assert(fixed < m_weights.size());
        std::ranges::fill(m_weights, 0);
        m_weights[fixed] = 1;
    }

    bool isValid() const
    {
        return std::ranges::count_if(m_weights, [](double d){return d < 0 || d > 1;}) == 0 && approximately::equals(std::accumulate(m_weights.begin(), m_weights.end(), 0.0, std::plus()), 1.0);
    }

    bool isNull() const
    {
        return std::ranges::find_if(m_weights, [](double d){return !approximately::equalsZero(d);}) == m_weights.end();
    }

    void clear()
    {
        std::ranges::fill(m_weights, 0);
    }

    void setWeight(size_t component, double weight)
    {
        assert(component < m_weights.size());
        m_weights[component] = weight;
    }

    void setWeights(const std::vector<double> & weights)
    {
        m_weights = weights;
    }

    double weight(size_t component) const
    {
        assert(component < m_weights.size());
        return m_weights[component];
    }

    void normalize() // todo what if contains negatives?
    {
        double total = totalWeight();
        if (total > 0) std::ranges::for_each(m_weights, [&total] (double & val) {val /= total;});
    }

    void normalize(double total)
    {
        normalize();
        std::ranges::for_each(m_weights, [&] (double & val) {val = val * total;});
    }

    double totalWeight() const
    {
        return std::accumulate(m_weights.begin(), m_weights.end(), 0.0, std::plus());;
    }

    bool isFixed() const
    {
        return std::ranges::find_if(m_weights, [](double d){return approximately::equals(d, 1.0);}) != m_weights.end();
    }

    template <typename ColorList>
        requires std::is_same_v<Color::Rgba, std::ranges::range_value_t<ColorList>>
    Color::Rgba rgba(const ColorList & colors) const
    {
        return Color::additiveMixture(colors, m_weights);
    }

    size_t dominantComponent() const
    {
        return std::ranges::max_element(m_weights) - m_weights.begin(); //todo need to check for equal elements?
    }

    const std::vector<double> & weights() const
    {
        return m_weights;
    }

private:

    std::vector<double> m_weights;
};

#endif // FUZZY_DROPLETS_FUZZYCOLOR_H
