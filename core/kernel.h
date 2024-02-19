#ifndef KERNEL_H
#define KERNEL_H

#include <concepts>
#include <numbers>

template <std::floating_point T>
struct Kernel
{
    virtual T weight(T pos, T location = 0, T scale = 1) const = 0;
    virtual ~Kernel() {}
};

template <std::floating_point T>
struct EpanechnikovKernel : public Kernel<T>
{
    inline T weight(T u, T location = 0, T scale = 1) const {return 0.75 * (T(1) - pow((u - location)/scale, 2)) / scale;}
};

template <std::floating_point T>
struct QuarticKernel : public Kernel<T>
{
    inline T weight(T u, T location = 0, T scale = 1) const {return 15 * pow(T(1) - pow((u - location)/scale, 2), 2) / (scale * 16);}
};

template <std::floating_point T>
struct TricubeKernel : public Kernel<T>
{
    inline T weight(T u, T location = 0, T scale = 1) const {return 70 * pow(T(1) - pow(std::abs((u - location)/scale),3), 3) / (scale * 81);}
};

template <std::floating_point T>
struct TriWeightKernel : public Kernel<T>
{
    inline T weight(T u, T location = 0, T scale = 1) const {return 35 * pow((1 - pow((u - location)/scale,2)), 3) / (scale * 32);}
};

template <std::floating_point T>
struct CosineKernel : public Kernel<T>
{
    inline T weight(T u, T location = 0, T scale = 1) const {return std::numbers::pi_v<T> * cos(std::numbers::pi_v<T> * ((u - location)/scale) / 2) / (4 * scale);}
};

#endif // KERNEL_H
