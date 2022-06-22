#include <microphysics.h>
#include <cmath>
#include <iostream>

namespace trt {
	
	/* See eq. (6, 8, 22), (45)Fouka & Ouichaoui (2014):
     * https://ui.adsabs.harvard.edu/abs/2014MNRAS.442..979F/abstract */
	trt::FP_Fouka::FP_Fouka(double p) {
		const double // Constants for interpolation of a1/a2/a4/b1 constants for given p.
		a10=-0.14602   ,a11= 3.62307e-2,a12=-5.76507e-3,a13= 3.46926e-4                ,
		a20=-0.36648   ,a21= 0.18031   ,a22=-7.30773e-2,a23= 1.12484e-2,a24=-6.17683e-4,
		a30= 9.69376e-2,a31=-0.48892   ,a32= 0.14024   ,a33=-1.93678e-2,a34= 1.01582e-3,
		b10=-0.20250   ,b11= 5.43462e-2,b12=-8.44171e-3,b13= 5.21281e-4                ;
		this->p = p;
		const double F_1 = M_PI * pow(2.0, 5.0/3.0) / sqrt(3.0) / tgamma(1.0/3.0);
		k_p  = 2.0*F_1/(p-1.0/3.0);
		C_p  = pow(2.0, (p+1.0)/2.0) / (p+1.0);
		C_p *= tgamma(p/4.0 + 19.0/12.0) * tgamma(p/4.0 -1.0/12.0);
		// Polynomial fit of a_x/b1 constants (eq. 21 Fouka (2014))
		a1 = a10 + a11*p + a12*pow(p,2.0) + a13*pow(p,3.0)                 ;
		a2 = a20 + a21*p + a22*pow(p,2.0) + a23*pow(p,3.0) + a24*pow(p,4.0);
		a3 = a30 + a31*p + a32*pow(p,2.0) + a33*pow(p,3.0) + a34*pow(p,4.0);
		b1 = b10 + b11*p + b12*pow(p,2.0) + b13*pow(p,3.0)                 ;
	}

	double trt::FP_Fouka::operator()(double x) const {
		/* Approximation: max error < 0.5% for 1 < p < 6 (eq. 22 Fouka 2014) */
		double exp1 = exp( a1 * x * x + a2 * x + a3 * pow(x,2.0/3.0) );
		double exp2 = pow( (1.0 - exp( b1 * x * x) ), p/5.0+1.0/2.0);
		return k_p*pow(x, 1.0/3.0)*exp1+C_p*pow(x,-(p-1.0)/2.0)*exp2;
	}

};

