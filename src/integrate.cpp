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
	double integrate_eort(std::function<AbsEm (double)> getAbsEm, double zmin, double zmax, double I_0=0) {
		// See: https://www.gnu.org/software/gsl/doc/html/ode-initval.html
		double h_start=0.1, eps_abs=0, eps_rel=1.e-3; // Intial stepsize, absolute, relative errors.
		gsl_odeiv2_system sys = {f, nullptr, 1, &getAbsEm}; // Derivative f, no jacobian, dim 1, abs/em source
		gsl_odeiv2_driver* d = gsl_odeiv2_driver_alloc_y_new(&sys, gsl_odeiv2_step_rk8pd, h_start, eps_abs, eps_rel);
		
		double I = I_0;
		int status = gsl_odeiv2_driver_apply (d, &zmin, zmax, &I);
		if(status != GSL_SUCCESS)
			 throw std::runtime_error("GSL threw error "+std::to_string(status)+" whilst integrating beam from zmin = "+std::to_string(zmin)+" to zmax = "+std::to_string(zmax));
		gsl_odeiv2_driver_free(d);
		
		return I;
	}
}

#endif
