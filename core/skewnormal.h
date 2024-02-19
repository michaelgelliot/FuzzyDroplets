#ifndef SKEWNORMAL_H
#define SKEWNORMAL_H

#include <cmath>
#include <numbers>

#include "core/toms462.h"

/* Mathematica code for multivariate skew normal

https://www.sciencedirect.com/science/article/pii/S0047259X08001152

MultivariateSkewNormalPDF[Y_, epsilon_, covmat_, skewness_] :=
 Module[{omega, delta},
  omega = covmat + (skewness Transpose[skewness]);
  delta = IdentityMatrix[Dimensions[Y][[1]]] - Transpose[skewness]*Inverse[omega]*skewness;
  Power[2, Dimensions[Y][[1]]] PDF[MultinormalDistribution[epsilon, omega], Y] * CDF[MultinormalDistribution[{0, 0}, delta], Transpose[skewness] . Inverse[omega] . (Y - epsilon)]
  ]

*/


// bivariate skew normal pdf
double skewNormalPDF(double x, double y, double ux, double uy, double sx, double sy, double rho, double s1, double s2, double s3, double s4)
{
    double sx2  = pow(sx,2);
    double sy2  = pow(sy,2);
    double s12  = pow(s1,2);
    double s22  = pow(s2,2);
    double s32  = pow(s3,2);
    double s42  = pow(s4,2);
    double rho2 = pow(rho,2);

    double rhosxsy = rho*sx*sy;
    double a = pow(s2*s3 + rhosxsy,2);
    double b = (s12 + sx2)*(s42 + sy2);

    double dsx = sqrt(1.0 - (s12*(s42 + sy2)) / (b-a));
    double dsy = sqrt(1.0 - (s42*(s12 + sx2)) / (b-a));
    double drho = -((s2*s3*(-(s2*s3) - rhosxsy))/((b-a) * sqrt(1.0 - (s42*(s12 + sx2))/(b-a))* sqrt(1.0 - (s12*(s42 + sy2))/(b-a))));
    double dd = (s22*s32 + 2*rho*s2*s3*sx*sy -s12*(s42 + sy2) -sx2*(s42 + sy2 - rho2*sy2));
    double dx = (s3*sx*(rho*sy*(x - ux) + sx*(uy - y)) + s12*s3*(uy - y) + s2*s3*(s3*(x - ux) + s1*(y - uy)) + s1*(s42*(ux - x) + sy*(sy*(ux - x) + rho*sx*(y - uy))))/dd;
    double dy = (s2*s42*(ux - x) + s2*s3*s4*(x - ux) + rho*s4*sx*sy*(x - ux) + s4*(s12 + sx2)*(uy - y) + s22*s3*(y - uy) + s2*sy*(sy*(ux - x) + rho*sx*(y - uy)))/dd;

    double result =(s42*pow(ux - x,2) + sy2*pow(ux - x,2) - 2*rhosxsy*(ux - x)*(uy - y) + (2*s2*s3*(x - ux) + (s12 + sx2)*(uy - y))*(uy - y))
                    /(2*s22*s32 + 4*rho*s2*s3*sx*sy -2*s12*(s42 + sy2) - 2*sx2*(s42 + sy2 - rho2*sy2));
    result = 2 * exp(result) * binormalCDF(dx,dy,0,0,dsx,dsy,drho);
    return result / (std::numbers::pi_v<double>*sqrt(s12 + sx2)*sqrt(s42 + sy2)*sqrt(1.0 - a/b));
}

#endif // SKEWNORMAL_H
