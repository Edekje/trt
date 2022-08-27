#ifndef TRT_INTEGRATE_H
#define TRT_INTEGRATE_H

#include <beam.h>
#include <hydro.h>
#include <functional>
#include <microphysics.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv2.h>

#include <string>
#include <stdexcept>
#include <cmath>
#include <iostream>

namespace trt {
	/* The function:
	 *
	 * std::function<AbsEm(double)> BindBeam(auto* hydrosim, auto* beam, Microphysics* MP, double nu)
	 *
	 * is wholly declared & defined in integrate.h since the use of an auto function parameter
	 * implicitly uses a template, which requires declaration and definiton to reside within the same
	 * compilaton unit */
	
	// One-off function used below by integrate_eort.	
	int f(double z, const double y[], double f[], void* params) {
		auto getAbsEm = static_cast<std::function<AbsEm (double)>*>(params);
		AbsEm AE = getAbsEm->operator()(z);
		f[0] = AE.em - AE.abs*y[0];
		return GSL_SUCCESS;
	}
	
	/* Solves the equation of radiative transfer
	  * dI/dx = eta - chi * I
	  * along the "bound" beam getAbsEm from zmin until zmax,
	  * and is sure to sample at z=0 (if in range).
	  * uses GSU, future integration methods
	  * will be defined in other functions/ */
	double integrate_eort(std::function<AbsEm (double)> getAbsEm, double zmin, double zmax, double dx, double I_0, double precision) {
		// See: https://www.gnu.org/software/gsl/doc/html/ode-initval.html
		double h_start=0.1, eps_abs=0, eps_rel=precision; // Intial stepsize, absolute, relative errors.
		gsl_odeiv2_system sys = {f, nullptr, 1, &getAbsEm}; // Derivative f, no jacobian, dim 1, abs/em source
		gsl_odeiv2_driver* d = gsl_odeiv2_driver_alloc_y_new(&sys, gsl_odeiv2_step_rkf45, h_start, eps_abs, eps_rel);
		
		double I = I_0;
		int status = gsl_odeiv2_driver_apply (d, &zmin, zmax-1e-6, &I);
		if(status != GSL_SUCCESS)
			 throw std::runtime_error("GSL threw error "+std::to_string(status)+" whilst integrating beam from zmin = "+std::to_string(zmin)+" to zmax = "+std::to_string(zmax));
		gsl_odeiv2_driver_free(d);
		
		return I;
	}
	
	AbsEm step_avg_eort(double I1, double z1, double z2, AbsEm AE1, AbsEm AE2){
		AbsEm avg( (AE1.abs+AE2.abs)/2.0, (AE1.em+AE2.em)/2.0 );
		double delta_z = z2-z1;
		// Case source function is infinite / no abs
		if(avg.abs == 0)
			return AbsEm(0, I1 + avg.em*delta_z);
		// Case standard pencil beam solution
		double optical_depth = avg.abs*delta_z;
		double S = avg.em / avg. abs;
		double I2 = I1*std::exp(-optical_depth) - std::expm1(-optical_depth)*S;
		
		return AbsEm(optical_depth, I2);
	}

	AbsEm integrate_eort_analytic(std::function<AbsEm (double)> getAbsEm, double zmin, double zmax,
			double dz_min,double dz_max, double variation_threshold,
			AbsEm (*step_f) (double, double, double, AbsEm, AbsEm), double I_0) {
		/*if(stepmode=="avg") {
			step_f = step_avg_eort;
		} else {
			throw std::invalid_argument("integrate_eort_analytic: "+stepmode+" is an invalid step mode.");
		}*/
		if(zmin > zmax) throw std::invalid_argument("integrate_eort_analytic: error, zmin > zmax.");
		
		// DEFINE RECURSIVE VARIABLE STEP-SIZE ODE SOLVER

		std::function<AbsEm(double,double,AbsEm,AbsEm,double)> recurse =
				[dz_min, dz_max, variation_threshold, step_f, getAbsEm, &recurse]
				(double z1, double z2, AbsEm AE1, AbsEm AE2, double I1) {
			double zmid;
			// used to see if a, b differ by more than a threshold, symmetrically.
			auto rel_diff = [](double a, double b) { return ( a==0 and b==0) ? 0 : std::abs( (a-b)/(a+b) ) * 2; };
			if(z1 < 0 and 0 < z2) {
				// 0 must be sampled to check if point closes to origin is shocked
				zmid = 0;
			} else if( z1+dz_max < z2 ) {
				// dz_max is the initial stepsize: no step shall be larger than that.
				zmid = (z1+z2)/2; // reduce recursion depth
			} else if( (rel_diff(AE1.abs, AE2.abs) > variation_threshold
				   or  rel_diff(AE1.em, AE2.em) > variation_threshold) and z2-z1 >= dz_min) {
				// If we have too much change, reduce stepsize, provided current stepsize exceeds minimum.
				zmid = (z1+z2)/2;
			} else {
				// Just propagate ahead.
				return step_f(I1, z1, z2, AE1, AE2);
			}
			// propagate other cases from z1 to mid and mid to z2.
			AbsEm AEmid = getAbsEm(zmid);
			AbsEm midresult = recurse(z1, zmid, AE1, AEmid, I1);
			double Imid = midresult.em;
			AbsEm endresult = recurse(zmid, z2, AEmid, AE2, Imid);
			double optical_depth_2 = midresult.abs + endresult.abs;
			double I2 = endresult.em;

			return AbsEm(optical_depth_2, I2);
		};
		// BEGIN RECURSION
		return recurse(zmin, zmax, getAbsEm(zmin), getAbsEm(zmax), I_0);
	}
}

#endif
