#ifndef FUZZY_DROPLETS_COLOR_H
#define FUZZY_DROPLETS_COLOR_H

#include <algorithm>
#include <cmath>
#include <ranges>
#include <array>
#include <cassert>
#include <QtGlobal>
#include <QtDebug>

#ifndef NDEBUG
#include "approximately.h"
#endif

namespace Color
{
using Rgba = unsigned int;

[[maybe_unused]] static constexpr int alpha(Rgba color) {return color >> 24;}
[[maybe_unused]] static constexpr int red(Rgba color)   {return ((color >> 16) & 0xffu);}
[[maybe_unused]] static constexpr int green(Rgba color) {return ((color >> 8) & 0xffu);}
[[maybe_unused]] static constexpr int blue(Rgba color)  {return (color & 0xffu);}

[[maybe_unused]] static constexpr Rgba rgba(int red, int green, int blue, int alpha = 255) {
    assert(red >= 0 && red <= 255);
    assert(green >= 0 && green <= 255);
    assert(blue >= 0 && blue <= 255);
    assert(alpha >= 0 && alpha <= 255);
    return ((alpha & 0xffu) << 24) | ((red & 0xffu) << 16) | ((green & 0xffu) << 8) | (blue & 0xffu);
}

[[maybe_unused]] static Rgba hsva(int hue, int saturation, int value, int alpha = 100) {
    assert(hue >= 0 && hue <= 360);
    assert(saturation >= 0 && saturation <= 100);
    assert(value >= 0 && value <= 100);
    assert(alpha >= 0 && alpha <= 100);
    double c = double(saturation * value) / 10000;
    double x = c*(1.0-abs(fmod((double)hue/60.0, 2)-1));
    double m = (double)value/100 - c;
    if (hue < 60)       return rgba(int((c+m)*255), int((x+m)*255), int(m*255), int(2.55*alpha));
    else if (hue < 120) return rgba(int((x+m)*255), int((c+m)*255), int(m*255), int(2.55*alpha));
    else if (hue < 180) return rgba(int(m*255), int((c+m)*255), int((x+m)*255), int(2.55*alpha));
    else if (hue < 240) return rgba(int(m*255), int((x+m)*255), int((c+m)*255), int(2.55*alpha));
    else if (hue < 300) return rgba(int((x+m)*255), int(m*255), int((c+m)*255), int(2.55*alpha));
    else                return rgba(int((c+m)*255), int(m*255), int((x+m)*255), int(2.55*alpha));
}

[[maybe_unused]] static constexpr std::array<int, 3> rgbComponents(Rgba color) {return {red(color), green(color), blue(color)};}
[[maybe_unused]] static constexpr std::array<int, 4> rgbaComponents(Rgba color) {return {red(color), green(color), blue(color), alpha(color)};}

[[nodiscard]] [[maybe_unused]] static constexpr Rgba setAlpha(Rgba color, int alpha) {assert (alpha >= 0 && alpha <= 255); return (color & ~(0xffu << 24)) | (alpha << 24);}
[[nodiscard]] [[maybe_unused]] static constexpr Rgba setRed(Rgba color, int red)     {assert (red >= 0 && red <= 255); return (color & ~(0xffu << 16)) | (red << 16);}
[[nodiscard]] [[maybe_unused]] static constexpr Rgba setGreen(Rgba color, int green) {assert (green >= 0 && green <= 255); return (color & ~(0xffu << 8)) | (green << 8);}
[[nodiscard]] [[maybe_unused]] static constexpr Rgba setBlue(Rgba color, int blue)   {assert (blue >= 0 && blue <= 255); return (color & ~0xffu) | blue;}

// given two colors, returns their additive mixture
template <typename FloatingPoint>
[[maybe_unused]] static constexpr Rgba additiveMixture(Rgba color1, Rgba color2, FloatingPoint weight1, FloatingPoint weight2)
    requires std::floating_point<FloatingPoint>
{
    assert(approximately::inRange(weight1, FloatingPoint(0), FloatingPoint(1)));
    assert(approximately::inRange(weight2, FloatingPoint(0), FloatingPoint(1)));
    assert(approximately::equals(weight1 + weight2, FloatingPoint(1.0)));
    return rgba(int(std::round(sqrt(pow(red(color1), 2) * weight1 + pow(red(color2), 2) * weight2))),
                int(std::round(sqrt(pow(green(color1), 2) * weight1 + pow(green(color2), 2) * weight2))),
                int(std::round(sqrt(pow(blue(color1), 2) * weight1 + pow(blue(color2), 2) * weight2))),
                int(std::round(alpha(color1) * weight1 + alpha(color2) * weight2)));
}

// given a range of colors (i.e. std::vector<Rgba>) and a range of weights that should sum to 1 (i.e. std::vector<double>), returns the additively mixed colour
template <std::ranges::range ColorContainer, std::ranges::range WeightsContainer>
[[maybe_unused]] static constexpr Rgba additiveMixture(const ColorContainer & colors, const WeightsContainer & weights)
    requires std::is_same_v<Rgba, std::ranges::range_value_t<ColorContainer>> && std::floating_point<std::ranges::range_value_t<WeightsContainer>>
{
    using FloatingPoint = typename WeightsContainer::value_type;
    assert(std::size(colors) > 0);
    assert(std::size(colors) == std::size(weights));
    //assert(approximately::equals(std::ranges::fold_left(weights, FloatingPoint(0), std::plus()), FloatingPoint(1.0)));

#ifndef Q_OS_WIN
    std::array<FloatingPoint, 4> mixed {0,0,0,0};
    for (size_t i = 0; i < colors.size(); ++i) {
        mixed[0] += pow(red(colors[i]), 2) * weights[i];
        mixed[1] += pow(green(colors[i]), 2) * weights[i];
        mixed[2] += pow(blue(colors[i]), 2) * weights[i];
        mixed[3] += alpha(colors[i]) * weights[i];
    }
#else
    auto mixed = std::ranges::fold_left(std::ranges::views::zip(colors, weights), std::array<FloatingPoint, 4>{0,0,0,0}, [](auto sum, const auto & elem)->std::array<FloatingPoint, 4>{
        assert(approximately::inRange(std::get<1>(elem), FloatingPoint(0), FloatingPoint(1)));
        return {sum[0] + pow(red(std::get<0>(elem)), 2) * std::get<1>(elem),
                sum[1] + pow(green(std::get<0>(elem)), 2) * std::get<1>(elem),
                sum[2] + pow(blue(std::get<0>(elem)), 2) * std::get<1>(elem),
                sum[3] + alpha(std::get<0>(elem)) * std::get<1>(elem)};});
#endif
    return rgba(int(std::round(sqrt(mixed[0]))), int(std::round(sqrt(mixed[1]))), int(std::round(sqrt(mixed[2]))), int(std::round(mixed[3])));
}

// given two colors, returns their alpha blended color
[[maybe_unused]] static  Rgba alphaBlend(Rgba lower, Rgba upper)
{
    double alphaLower = (double)alpha(lower) / 255;
    double alphaUpper = (double)alpha(upper) / 255;
    double alpha0 = alphaUpper + alphaLower * (1.0 - alphaUpper);
    return rgba(int(std::round((red(upper) * alphaUpper + red(lower) * alphaLower * (1.0 - alphaUpper)) / alpha0)),
                int(std::round((green(upper) * alphaUpper + green(lower) * alphaLower * (1.0 - alphaUpper)) / alpha0)),
                int(std::round((blue(upper) * alphaUpper + blue(lower) * alphaLower * (1.0 - alphaUpper)) / alpha0)),
                int(std::round(alpha0 * 255)));
}

// given a range of colors (i.e. std::vector<Color>), ordered according to a z stack with the bottom first and the top last, returns their alpha blended color
template <std::ranges::range ColorContainer>
[[maybe_unused]] static constexpr Rgba alphaBlend(const ColorContainer & colors)
    requires std::is_same_v<Rgba, std::ranges::range_value_t<ColorContainer>>
{
    assert(std::size(colors) > 0);
    auto first = rgbaComponents(*std::begin(colors));

#ifndef Q_OS_WIN
    std::array<double, 4> result {(double)first[0], (double)first[1], (double)first[2], (double)first[3]};
    for (size_t i = 1; i < colors.size(); ++i) {
        auto upper = colors[i];
        double alphaLower = (double)result[3] / 255;
        double alphaUpper = (double)alpha(upper) / 255;
        double alpha0 = alphaUpper + alphaLower * (1.0 - alphaUpper);
        result = {(red(upper) * alphaUpper + result[0] * alphaLower * (1.0 - alphaUpper)) / alpha0,
                  (green(upper) * alphaUpper + result[1] * alphaLower * (1.0 - alphaUpper)) / alpha0,
                  (blue(upper) * alphaUpper + result[2] * alphaLower * (1.0 - alphaUpper)) / alpha0,
                  alpha0 * 255};
    }
#else
    std::array<double, 4> init {(double)first[0], (double)first[1], (double)first[2], (double)first[3]};
    auto result = std::ranges::fold_left(std::ranges::views::drop(colors, 1), init, [](auto lower, Rgba upper) -> std::array<double, 4> {
        double alphaLower = (double)lower[3] / 255;
        double alphaUpper = (double)alpha(upper) / 255;
        double alpha0 = alphaUpper + alphaLower * (1.0 - alphaUpper);
        return {(red(upper) * alphaUpper + lower[0] * alphaLower * (1.0 - alphaUpper)) / alpha0,
                (green(upper) * alphaUpper + lower[1] * alphaLower * (1.0 - alphaUpper)) / alpha0,
                (blue(upper) * alphaUpper + lower[2] * alphaLower * (1.0 - alphaUpper)) / alpha0,
                alpha0 * 255};
    });
#endif
    return rgba(int(std::round(result[0])),
                int(std::round(result[1])),
                int(std::round(result[2])),
                int(std::round(result[3])));
}

// named colors from CSS3, plus "transparent"
namespace named
{
[[maybe_unused]] static constexpr Rgba aliceblue            = setAlpha(0xf0f8ff,255);
[[maybe_unused]] static constexpr Rgba antiquewhite         = setAlpha(0xfaebd7,255);
[[maybe_unused]] static constexpr Rgba aqua                 = setAlpha(0x00ffff,255);
[[maybe_unused]] static constexpr Rgba aquamarine           = setAlpha(0x7fffd4,255);
[[maybe_unused]] static constexpr Rgba azure                = setAlpha(0xf0ffff,255);
[[maybe_unused]] static constexpr Rgba beige                = setAlpha(0xf5f5dc,255);
[[maybe_unused]] static constexpr Rgba bisque               = setAlpha(0xffe4c4,255);
[[maybe_unused]] static constexpr Rgba black                = setAlpha(0x000000,255);
[[maybe_unused]] static constexpr Rgba blanchedalmond       = setAlpha(0xffebcd,255);
[[maybe_unused]] static constexpr Rgba blue                 = setAlpha(0x0000ff,255);
[[maybe_unused]] static constexpr Rgba blueviolet           = setAlpha(0x8a2be2,255);
[[maybe_unused]] static constexpr Rgba brown                = setAlpha(0xa52a2a,255);
[[maybe_unused]] static constexpr Rgba burlywood            = setAlpha(0xdeb887,255);
[[maybe_unused]] static constexpr Rgba cadetblue            = setAlpha(0x5f9ea0,255);
[[maybe_unused]] static constexpr Rgba chartreuse           = setAlpha(0x7fff00,255);
[[maybe_unused]] static constexpr Rgba chocolate            = setAlpha(0xd2691e,255);
[[maybe_unused]] static constexpr Rgba coral                = setAlpha(0xff7f50,255);
[[maybe_unused]] static constexpr Rgba cornflowerblue       = setAlpha(0x6495ed,255);
[[maybe_unused]] static constexpr Rgba cornsilk             = setAlpha(0xfff8dc,255);
[[maybe_unused]] static constexpr Rgba crimson              = setAlpha(0xdc143c,255);
[[maybe_unused]] static constexpr Rgba cyan                 = setAlpha(0x00ffff,255);
[[maybe_unused]] static constexpr Rgba darkblue             = setAlpha(0x00008b,255);
[[maybe_unused]] static constexpr Rgba darkcyan             = setAlpha(0x008b8b,255);
[[maybe_unused]] static constexpr Rgba darkgoldenrod        = setAlpha(0xb8860b,255);
[[maybe_unused]] static constexpr Rgba darkgray             = setAlpha(0xa9a9a9,255);
[[maybe_unused]] static constexpr Rgba darkgreen            = setAlpha(0x006400,255);
[[maybe_unused]] static constexpr Rgba darkkhaki            = setAlpha(0xbdb76b,255);
[[maybe_unused]] static constexpr Rgba darkmagenta          = setAlpha(0x8b008b,255);
[[maybe_unused]] static constexpr Rgba darkolivegreen       = setAlpha(0x556b2f,255);
[[maybe_unused]] static constexpr Rgba darkorange           = setAlpha(0xff8c00,255);
[[maybe_unused]] static constexpr Rgba darkorchid           = setAlpha(0x9932cc,255);
[[maybe_unused]] static constexpr Rgba darkred              = setAlpha(0x8b0000,255);
[[maybe_unused]] static constexpr Rgba darksalmon           = setAlpha(0xe9967a,255);
[[maybe_unused]] static constexpr Rgba darkseagreen         = setAlpha(0x8fbc8f,255);
[[maybe_unused]] static constexpr Rgba darkslateblue        = setAlpha(0x483d8b,255);
[[maybe_unused]] static constexpr Rgba darkslategray        = setAlpha(0x2f4f4f,255);
[[maybe_unused]] static constexpr Rgba darkturquoise        = setAlpha(0x00ced1,255);
[[maybe_unused]] static constexpr Rgba darkviolet           = setAlpha(0x9400d3,255);
[[maybe_unused]] static constexpr Rgba deeppink             = setAlpha(0xff1493,255);
[[maybe_unused]] static constexpr Rgba deepskyblue          = setAlpha(0x00bfff,255);
[[maybe_unused]] static constexpr Rgba dimgray              = setAlpha(0x696969,255);
[[maybe_unused]] static constexpr Rgba dodgerblue           = setAlpha(0x1e90ff,255);
[[maybe_unused]] static constexpr Rgba firebrick            = setAlpha(0xb22222,255);
[[maybe_unused]] static constexpr Rgba floralwhite          = setAlpha(0xfffaf0,255);
[[maybe_unused]] static constexpr Rgba forestgreen          = setAlpha(0x228b22,255);
[[maybe_unused]] static constexpr Rgba fuchsia              = setAlpha(0xff00ff,255);
[[maybe_unused]] static constexpr Rgba gainsboro            = setAlpha(0xdcdcdc,255);
[[maybe_unused]] static constexpr Rgba ghostwhite           = setAlpha(0xf8f8ff,255);
[[maybe_unused]] static constexpr Rgba gold                 = setAlpha(0xffd700,255);
[[maybe_unused]] static constexpr Rgba goldenrod            = setAlpha(0xdaa520,255);
[[maybe_unused]] static constexpr Rgba gray                 = setAlpha(0x808080,255);
[[maybe_unused]] static constexpr Rgba green                = setAlpha(0x008000,255);
[[maybe_unused]] static constexpr Rgba greenyellow          = setAlpha(0xadff2f,255);
[[maybe_unused]] static constexpr Rgba honeydew             = setAlpha(0xf0fff0,255);
[[maybe_unused]] static constexpr Rgba hotpink              = setAlpha(0xff69b4,255);
[[maybe_unused]] static constexpr Rgba indianred            = setAlpha(0xcd5c5c,255);
[[maybe_unused]] static constexpr Rgba indigo               = setAlpha(0x4b0082,255);
[[maybe_unused]] static constexpr Rgba ivory                = setAlpha(0xfffff0,255);
[[maybe_unused]] static constexpr Rgba khaki                = setAlpha(0xf0e68c,255);
[[maybe_unused]] static constexpr Rgba lavender             = setAlpha(0xe6e6fa,255);
[[maybe_unused]] static constexpr Rgba lavenderblush        = setAlpha(0xfff0f5,255);
[[maybe_unused]] static constexpr Rgba lawngreen            = setAlpha(0x7cfc00,255);
[[maybe_unused]] static constexpr Rgba lemonchiffon         = setAlpha(0xfffacd,255);
[[maybe_unused]] static constexpr Rgba lightblue            = setAlpha(0xadd8e6,255);
[[maybe_unused]] static constexpr Rgba lightcoral           = setAlpha(0xf08080,255);
[[maybe_unused]] static constexpr Rgba lightcyan            = setAlpha(0xe0ffff,255);
[[maybe_unused]] static constexpr Rgba lightgoldenrodyellow = setAlpha(0xfafad2,255);
[[maybe_unused]] static constexpr Rgba lightgray            = setAlpha(0xd3d3d3,255);
[[maybe_unused]] static constexpr Rgba lightgreen           = setAlpha(0x90ee90,255);
[[maybe_unused]] static constexpr Rgba lightpink            = setAlpha(0xffb6c1,255);
[[maybe_unused]] static constexpr Rgba lightsalmon          = setAlpha(0xffa07a,255);
[[maybe_unused]] static constexpr Rgba lightseagreen        = setAlpha(0x20b2aa,255);
[[maybe_unused]] static constexpr Rgba lightskyblue         = setAlpha(0x87cefa,255);
[[maybe_unused]] static constexpr Rgba lightslategray       = setAlpha(0x778899,255);
[[maybe_unused]] static constexpr Rgba lightsteelblue       = setAlpha(0xb0c4de,255);
[[maybe_unused]] static constexpr Rgba lightyellow          = setAlpha(0xffffe0,255);
[[maybe_unused]] static constexpr Rgba lime                 = setAlpha(0x00ff00,255);
[[maybe_unused]] static constexpr Rgba limegreen            = setAlpha(0x32cd32,255);
[[maybe_unused]] static constexpr Rgba linen                = setAlpha(0xfaf0e6,255);
[[maybe_unused]] static constexpr Rgba magenta              = setAlpha(0xff00ff,255);
[[maybe_unused]] static constexpr Rgba maroon               = setAlpha(0x800000,255);
[[maybe_unused]] static constexpr Rgba mediumaquamarine     = setAlpha(0x66cdaa,255);
[[maybe_unused]] static constexpr Rgba mediumblue           = setAlpha(0x0000cd,255);
[[maybe_unused]] static constexpr Rgba mediumorchid         = setAlpha(0xba55d3,255);
[[maybe_unused]] static constexpr Rgba mediumpurple         = setAlpha(0x9370db,255);
[[maybe_unused]] static constexpr Rgba mediumseagreen       = setAlpha(0x3cb371,255);
[[maybe_unused]] static constexpr Rgba mediumslateblue      = setAlpha(0x7b68ee,255);
[[maybe_unused]] static constexpr Rgba mediumspringgreen    = setAlpha(0x00fa9a,255);
[[maybe_unused]] static constexpr Rgba mediumturquoise      = setAlpha(0x48d1cc,255);
[[maybe_unused]] static constexpr Rgba mediumvioletred      = setAlpha(0xc71585,255);
[[maybe_unused]] static constexpr Rgba midnightblue         = setAlpha(0x191970,255);
[[maybe_unused]] static constexpr Rgba mintcream            = setAlpha(0xf5fffa,255);
[[maybe_unused]] static constexpr Rgba mistyrose            = setAlpha(0xffe4e1,255);
[[maybe_unused]] static constexpr Rgba moccasin             = setAlpha(0xffe4b5,255);
[[maybe_unused]] static constexpr Rgba navajowhite          = setAlpha(0xffdead,255);
[[maybe_unused]] static constexpr Rgba navy                 = setAlpha(0x000080,255);
[[maybe_unused]] static constexpr Rgba oldlace              = setAlpha(0xfdf5e6,255);
[[maybe_unused]] static constexpr Rgba olive                = setAlpha(0x808000,255);
[[maybe_unused]] static constexpr Rgba olivedrab            = setAlpha(0x6b8e23,255);
[[maybe_unused]] static constexpr Rgba orange               = setAlpha(0xffa500,255);
[[maybe_unused]] static constexpr Rgba orangered            = setAlpha(0xff4500,255);
[[maybe_unused]] static constexpr Rgba orchid               = setAlpha(0xda70d6,255);
[[maybe_unused]] static constexpr Rgba palegoldenrod        = setAlpha(0xeee8aa,255);
[[maybe_unused]] static constexpr Rgba palegreen            = setAlpha(0x98fb98,255);
[[maybe_unused]] static constexpr Rgba paleturquoise        = setAlpha(0xafeeee,255);
[[maybe_unused]] static constexpr Rgba palevioletred        = setAlpha(0xdb7093,255);
[[maybe_unused]] static constexpr Rgba papayawhip           = setAlpha(0xffefd5,255);
[[maybe_unused]] static constexpr Rgba peachpuff            = setAlpha(0xffdab9,255);
[[maybe_unused]] static constexpr Rgba peru                 = setAlpha(0xcd853f,255);
[[maybe_unused]] static constexpr Rgba pink                 = setAlpha(0xffc0cb,255);
[[maybe_unused]] static constexpr Rgba plum                 = setAlpha(0xdda0dd,255);
[[maybe_unused]] static constexpr Rgba powderblue           = setAlpha(0xb0e0e6,255);
[[maybe_unused]] static constexpr Rgba purple               = setAlpha(0x800080,255);
[[maybe_unused]] static constexpr Rgba red                  = setAlpha(0xff0000,255);
[[maybe_unused]] static constexpr Rgba rosybrown            = setAlpha(0xbc8f8f,255);
[[maybe_unused]] static constexpr Rgba royalblue            = setAlpha(0x4169e1,255);
[[maybe_unused]] static constexpr Rgba saddlebrown          = setAlpha(0x8b4513,255);
[[maybe_unused]] static constexpr Rgba salmon               = setAlpha(0xfa8072,255);
[[maybe_unused]] static constexpr Rgba sandybrown           = setAlpha(0xf4a460,255);
[[maybe_unused]] static constexpr Rgba seagreen             = setAlpha(0x2e8b57,255);
[[maybe_unused]] static constexpr Rgba seashell             = setAlpha(0xfff5ee,255);
[[maybe_unused]] static constexpr Rgba sienna               = setAlpha(0xa0522d,255);
[[maybe_unused]] static constexpr Rgba silver               = setAlpha(0xc0c0c0,255);
[[maybe_unused]] static constexpr Rgba skyblue              = setAlpha(0x87ceeb,255);
[[maybe_unused]] static constexpr Rgba slateblue            = setAlpha(0x6a5acd,255);
[[maybe_unused]] static constexpr Rgba slategray            = setAlpha(0x708090,255);
[[maybe_unused]] static constexpr Rgba snow                 = setAlpha(0xfffafa,255);
[[maybe_unused]] static constexpr Rgba springgreen          = setAlpha(0x00ff7f,255);
[[maybe_unused]] static constexpr Rgba steelblue            = setAlpha(0x4682b4,255);
[[maybe_unused]] static constexpr Rgba tan                  = setAlpha(0xd2b48c,255);
[[maybe_unused]] static constexpr Rgba teal                 = setAlpha(0x008080,255);
[[maybe_unused]] static constexpr Rgba thistle              = setAlpha(0xd8bfd8,255);
[[maybe_unused]] static constexpr Rgba tomato               = setAlpha(0xff6347,255);
[[maybe_unused]] static constexpr Rgba transparent          = 0;
[[maybe_unused]] static constexpr Rgba turquoise            = setAlpha(0x40e0d0,255);
[[maybe_unused]] static constexpr Rgba violet               = setAlpha(0xee82ee,255);
[[maybe_unused]] static constexpr Rgba wheat                = setAlpha(0xf5deb3,255);
[[maybe_unused]] static constexpr Rgba white                = setAlpha(0xffffff,255);
[[maybe_unused]] static constexpr Rgba whitesmoke           = setAlpha(0xf5f5f5,255);
[[maybe_unused]] static constexpr Rgba yellow               = setAlpha(0xffff00,255);
[[maybe_unused]] static constexpr Rgba yellowgreen          = setAlpha(0x9acd32,255);
}
}

#endif // FUZZY_DROPLETS_COLOR_H
