# Sunny-d

Sunny-d provides a super convenient Matlab interface to the [SUNDIALS](http://computation.llnl.gov/projects/sundials-suite-nonlinear-differential-algebraic-equation-solvers) ODE solvers. These can be 100x faster than Matlab's solvers, particularly for stiff problems.

## Usage

There are two ways to invoke `sunnyd`. 

Firstly, as a drop-in replacement for `ode45`, `ode15s` etc. using an update function written in Matlab, e.g.

		[t, Y] = sunnyd(@dydt, T, x0, theta)

where `@dydt` looks something like:

	function d = dydt(t,y,theta)
	    a = theta(1);
	    b = theta(2);
	    d = [y(2); -a * (y(2) * (y(1) * y(1) - b) + y(1)) ];   
	end

Further, you can seamlessly incorporate an update function written in C

		[t, Y] = sunnyd('dydt.c', T, x0, theta)

where `dydt.c` looks something like:

	#include "sunnyd.h"
	
	void dydt(double t, double *y, double *ydot, double *theta)
	{
	    double a = theta[0];
	    double b = theta[1];    
	
	    ydot[0] = y[1]; 
	    ydot[1] =  -a * (y[1] * (y[0] * y[0] - b) + y[0]);
	}

That's pretty much it. Sunnyd is smart enough to transparently recompile, only when needed. Most of the magic lives in `sunnyd.h` which can be readily modified to suit particular numerical needs.

# Installation

Clone this repository and add it to your Matlab path.

