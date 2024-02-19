#ifndef PHYLOGENERICS_UTILITY_TRIM_STRING_VIEW_HPP
#define PHYLOGENERICS_UTILITY_TRIM_STRING_VIEW_HPP

#include <string_view>
#include <cctype>

std::string_view trim(std::string_view view)
{
    if (view.size() == 0)
        return view;
    size_t leftPos = 0;
    size_t rightPos = view.size() - 1;
    while (leftPos < view.size() && std::isspace(view[leftPos]))
        ++leftPos;
    while (rightPos > leftPos && rightPos != 0 && std::isspace(view[rightPos]))
        --rightPos;
    return view.substr(leftPos, rightPos - leftPos + 1);
}

#endif // PHYLOGENERICS_UTILITY_TRIM_STRING_VIEW_HPP
