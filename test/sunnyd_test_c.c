
#include "sunnyd.h"

void dydt(double t, double *y, double *ydot, double *p)
{
    double a = p[0];
    double b = p[1];

    ydot[0] = y[1]; 
    ydot[1] =  -a * (y[1] * (y[0] * y[0] - b) + y[0]);
}


     