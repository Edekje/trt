#ifndef TRT_INTEGRATE_H
#define TRT_INTEGRATE_H

#include <beam.h>
#include <hydro.h>
#include <microphysics.h>
#include <relativity.h>

#include <functional>
#include <string>
#include <iostream>

namespace trt {
	
	/* Bind Beam binds a Beam, Microphysics specification,
	 * and HydroSim together to produce a functor: double z -> AbsEm AE
	 * which has the domain [beam.zmin,beam.zmax].
	 * This may be integrated over by a preferred routine from
	 * integrate.h .
	 *
	 * This function is wholly declared & defined
	 * in integrate.h since the use of an auto function parameter
	 * implicitly uses a template, which requires declaration and definiton
	 * to reside within the same compilaton unit */
	std::function<AbsEm(double)> BindBeam(auto* hydrosim, auto* beam, Microphysics* MP, double nu, double cutoff=0) {
		return [hydrosim, beam, MP, nu, cutoff] (double z) {
			auto coordinate = beam->operator()(z);
			double cos_theta = z / coordinate.r;
			auto rest_hydro_var = hydrosim->getHydroVar(coordinate);
			// If internal energy : density ratio is below cutoff, it is considered
			// unshocked and non-radiating.
			if(rest_hydro_var.e_th / rest_hydro_var.rho < cutoff) return AbsEm(1.0e-50,1.0e-50);
			// hacky fix;
			// In the middle, no velocity - avoid divide by 0 in cos_theta -> set df 1.
			double df = (coordinate.r!=0) ? doppler_factor(rest_hydro_var.u1, cos_theta) : 1;
			double nu_prime = nu / df; // frequency in fluid frame.
			
		   return boostAbsEmToLab( MP->getAbsEm(rest_hydro_var, nu_prime), df); };
	}

	/* Solves the equation of radiative transfer
	  * dI/dx = eta - chi * I
	  * along the "bound" beam getAbsEm from zmin until zmax,
	  * and is sure to sample at z=0 (if in range).
	  * uses GSL, future integration methods
	  * will be defined in other functions/ */
	double integrate_eort(std::function<AbsEm (double)> getAbsEm, double zmin, double zmax, double dx, double I_0=0, double precision=0.001);
	
	/* Steps ahead from z1 to z2 by solving the eort for constant AbsEm,
	 * averages AbsEm at z1 & 2 for this purpose. Returns specific
	 * intensity (AbsEm.em), and optical thickness (AbsEm.abs) */
	AbsEm step_avg_eort(double I1, double z1, double z2, AbsEm AE1, AbsEm AE2);

	AbsEm integrate_eort_analytic(std::function<AbsEm (double)> getAbsEm, double zmin, double zmax, double dz_min, double dz_max, double variation_threshold, AbsEm (*step_f) (double, double, double, AbsEm, AbsEm), double I_0);

}

#endif
