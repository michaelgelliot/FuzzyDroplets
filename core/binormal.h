#ifndef FUZZYDROPLETS_CORE_BINORMAL_H
#define FUZZYDROPLETS_CORE_BINORMAL_H

#include <cmath>
#include <numbers>
#include <array>

class BinormalDistribution
{
public:

    BinormalDistribution() {}
    BinormalDistribution(double ux, double uy, double sx, double sy, double r) : m_ux(ux), m_uy(uy), m_sx(sx), m_sy(sy), m_r(r) {}

    void setMean(double x, double y) { m_ux = x; m_uy = y; }
    void setMeanX(double x) {m_ux = x;}
    void setMeanY(double y) {m_uy = y;}
    void setStdDev(double sx, double sy) {m_sx = sx; m_sy = sy;}
    void setStdDevX(double sx) {m_sx = sx;}
    void setStdDevY(double sy) {m_sy = sy;}
    void setRho(double r) {m_r = r;}

    double meanX() const {return m_ux;}
    double meanY() const {return m_uy;}
    double stdDevX() const {return m_sx;}
    double stdDevY() const {return m_sy;}
    double rho() const {return m_r;}

    double logPdf(double x, double y) const
    {
        return (pow(m_sy,2)*pow(m_ux - x,2) - 2*m_r*m_sx*m_sy*(m_ux - x)*(m_uy - y) + pow(m_sx,2)*pow(m_uy - y,2)) / (2*(pow(m_r,2) - 1)*pow(m_sx,2)*pow(m_sy,2)) - log(2*sqrt(1.0 - pow(m_r,2))*m_sx*m_sy*std::numbers::pi_v<double>);
    }

    double pdf(double x, double y) const
    {
        return exp((pow(m_sy,2)*pow(m_ux - x,2) - 2*m_r*m_sx*m_sy*(m_ux - x)*(m_uy - y) + pow(m_sx,2)*pow(m_uy - y,2)) / (2*(pow(m_r,2)-1)*pow(m_sx,2)*pow(m_sy,2))) / (2*sqrt(1.0 - pow(m_r,2))*m_sx*m_sy*std::numbers::pi_v<double>);
    }

    std::array<double, 4> covMat() const
    {
        double r = m_sx * m_sy * m_r;
        return {pow(m_sx, 2), r, r, pow(m_sy, 2)};
    }

    std::array<double, 4> inverseCovMat() const
    {
        double r = -(m_r/(m_sx*m_sy - pow(m_r,2)*m_sx*m_sy));
        return {1.0/(pow(m_sx,2) - pow(m_r,2)*pow(m_sx,2)), r, r, 1.0/(pow(m_sy,2) - pow(m_r,2)*pow(m_sy,2))};
    }

    // returns the squared mahalanobis distance from x,y to the distribution
    double mahalanobisDistance(double x, double y) const
    {
        return -((pow(m_sy,2) * pow(m_ux - x,2) - 2 * m_r * m_sx * m_sy * (m_ux - x) * (m_uy - y) + pow(m_sx,2) * pow(m_uy - y,2)) / ((pow(m_r,2) - 1) * pow(m_sx,2) * pow(m_sy,2)));
    }

private:

    double m_ux {0};
    double m_uy {0};
    double m_sx {1};
    double m_sy {1};
    double m_r {0};
};

#endif // FUZZYDROPLETS_CORE_BINORMAL_H
