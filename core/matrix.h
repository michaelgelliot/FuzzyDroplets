#ifndef FUZZYDROPLETS_CORE_MATRIX_H
#define FUZZYDROPLETS_CORE_MATRIX_H

#include <array>

// functions for converting between 2D matrix coordinates (x,y) and the corresponding position in a flat list (index)

size_t index(size_t x, size_t y, size_t width)
{
    return y * width + x;
}

std::array<size_t, 2> coordinates(size_t index, size_t width)
{
    return {index % width, index / width};
}

#endif // FUZZYDROPLETS_CORE_MATRIX_H
