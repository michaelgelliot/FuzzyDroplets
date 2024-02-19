#ifndef FUZZY_DROPLETS_ROUND_H
#define FUZZY_DROPLETS_ROUND_H

// round n down to the nearest multiple of m
long roundDown(long n, long m)
{
    return n >= 0 ? (n / m) * m : ((n - m + 1) / m) * m;
}

// round n up to the nearest multiple of m
long roundUp(long n, long m)
{
    return n >= 0 ? ((n + m - 1) / m) * m : (n / m) * m;
}

// round n up or down to the nearest multiple of m
long roundNearest(long n, long m)
{
    auto up = roundUp(n, m);
    auto down = roundDown(n, m);
    return (up - n <= n - down) ? up : down;
}

#endif // FUZZY_DROPLETS_ROUND_H
