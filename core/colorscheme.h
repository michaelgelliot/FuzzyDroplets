#ifndef FUZZY_DROPLETS_COLORSCHEME_H
#define FUZZY_DROPLETS_COLORSCHEME_H

#include <cstddef>
#include <vector>
#include <random>
#include "color.h"

class ColorScheme
{
public:

    ColorScheme()
        : m_colors {Color::named::black,
                    Color::named::yellowgreen,
                    Color::named::deepskyblue,
                    Color::named::blueviolet,
                    Color::named::crimson,
                    Color::named::forestgreen,
                    Color::named::royalblue,
                    Color::named::deeppink,
                    Color::named::darkorange,
                    Color::named::lightgreen,
                    Color::named::darkblue,
                    Color::named::gold,
                    Color::named::indigo,
                    Color::named::lightseagreen,
                    Color::named::steelblue,
                    Color::named::tomato}
    {}

    Color::Rgba color(size_t i) const
    {
        assert(i >= 0);
        if (i <= m_colors.size())
            enlargeColorList(i);
        return m_colors[i];
    }

    int indexOf(Color::Rgba color)
    {
        auto it = std::find(m_colors.begin(), m_colors.end(), color);
        return (it == m_colors.end() ? -1 : (int)(it - m_colors.begin()));
    }

    void setColor(int i, Color::Rgba color)
    {
        assert(i >= 0);
        if (i <= m_colors.size())
            enlargeColorList(i);
        m_colors[i] = color;
    }

    auto colors(size_t firstIndex, size_t lastIndex) const{
        assert(lastIndex >= firstIndex);
        if (lastIndex >= m_colors.size() - 1)
            enlargeColorList(lastIndex);
        return std::ranges::views::drop(std::ranges::views::take(m_colors, lastIndex + 1), firstIndex);
    }

private:

    void enlargeColorList(size_t maxIndex) const
    {
        for (size_t j = m_colors.size(); j <= maxIndex; ++j) {
            std::mt19937 gen(static_cast<unsigned int>(j));
            m_colors.push_back(Color::hsva(m_hDist(gen), m_sDist(gen), m_vDist(gen), 100));
        }
    }

    mutable std::vector<Color::Rgba> m_colors;
    mutable std::uniform_int_distribution<> m_hDist{0, 359};
    mutable std::uniform_int_distribution<> m_sDist{40, 100};
    mutable std::uniform_int_distribution<> m_vDist{60, 100};
};

#endif // FUZZY_DROPLETS_COLORSCHEME_H
