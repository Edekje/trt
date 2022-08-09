#include <microphysics.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <globals.h>
		
#define CHECKSET(XXX) if(param.isset("XXX")) XXX = param.getDouble("XXX"); else throw std::invalid_argument("Necessary argument -XXX to CS_Microphysics is missing.")

namespace trt {
	
	/* See eq. (6, 8, 22), (45)Fouka & Ouichaoui (2014):
     * https://ui.adsabs.harvard.edu/abs/2014MNRAS.442..979F/abstract */
	FP_Fouka::FP_Fouka(double p) {
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

	double FP_Fouka::operator()(double x) const {
		/* Approximation: max error < 0.5% for 1 < p < 6 (eq. 22 Fouka 2014) */
		double exp1 = exp( a1 * x * x + a2 * x + a3 * pow(x,2.0/3.0) );
		double exp2 = pow( (1.0 - exp( b1 * x * x) ), p/5.0+1.0/2.0);
		return k_p*pow(x, 1.0/3.0)*exp1+C_p*pow(x,-(p-1.0)/2.0)*exp2;
	}
	
	CS_Microphysics::CS_Microphysics(Config& param) {
		// Checks if following parameters are present & sets them or throws exception.
		p = param.getDouble("p");
		e_e = param.getDouble("e_e");
		e_b = param.getDouble("e_b");
		electron_fraction = param.getDouble("electron_fraction");
		M = param.getDouble("M");
		L = param.getDouble("L");
		FP_Fouka temp1(p), temp2(p+1);
		FP1 = temp1;
		FP2 = temp2;
	}
	
	AbsEm CS_Microphysics::getAbsEm(HydroVar HV, double nu) {
		// B = c sqrt(8 pi rho*(gamma-1))
		// rho (gamma-1) = e - rho = p / (adiabindex-1) = e_th
		double gamma_pmin1 = HV.e_th / HV.rho;
		double gamma_1 = (p-2)/(p-1) * M_PROTON / M_ELECTRON * e_e / electron_fraction * gamma_pmin1; //gamma_m
		double B = C_LIGHT*sqrt(8*M_PI*e_b*HV.e_th * M * pow(L, -3) ); // B field in gauss
		double nu_larmor = B*Q_ELECTRON/2.0/M_PI/M_ELECTRON/C_LIGHT;
		double C = (p-1)*HV.rho*M*pow(L,-3)/M_PROTON/electron_fraction; // gamma_1^(P-1) cancels w/ P_1 expr.
		double P_1 = M_PI * sqrt(3)*pow(Q_ELECTRON, 2)*nu_larmor * C / C_LIGHT;
		double nu_1 = 3.0/2.0*pow(gamma_1,2)*nu_larmor;
		double x = nu / nu_1;
		
		double em_coeff = P_1 * FP1(x) / 4.0 / M_PI; // in erg / s / Hz / cm / sr
		double abs_coeff = P_1 / gamma_1 * pow(x,-2) * FP2(x); // 1 / cm

		AbsEm returnme(abs_coeff*L, em_coeff*L); // convert to units of L^-1, erg sr^-1 cm^-2 L^-1
		return returnme;
	}
}

