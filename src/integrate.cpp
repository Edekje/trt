#ifndef TRT_INTEGRATE_H
#define TRT_INTEGRATE_H

#include <beam.h>
#include <hydro.h>
#include <functional>
#include <microphysics.h>

namespace trt {
	/* The function:
	 *
	 * std::function<AbsEm(double)> BindBeam(auto* hydrosim, auto* beam, Microphysics* MP, double nu)
	 *
	 * is wholly declared & defined in integrate.h since the use of an auto function parameter
	 * implicitly uses a template, which requires declaration and definiton to reside within the same
	 * compilaton unit */
	
	/* Solves the equation of radiative transfer
	  * dI/dx = eta - chi * I
	  * along the "bound" beam getAbsEm from zmin until zmax,
	  * and is sure to sample at z=0 (if in range).
	  * uses GSU, future integration methods
	  * will be defined in other functions/ */
	double integrate_eort(std::function<AbsEm (double)> getAbsEm);
}

#endif
