#ifndef TOMS462_H
#define TOMS462_H

double bivnor ( double ah, double ak, double r );
double gauss ( double t );

// wrapper function added by Mick, bivariate normal cdf with mean {mux, muy} standard deviation {sx,xy}, correlation coefficient rho
double binormalCDF(double x, double y, double mux, double muy, double s1, double s2, double rho);

#endif // TOMS462_H
