#ifndef TRT_INTEGRATE_H
#define TRT_INTEGRATE_H

#include <beam.h>
#include <hydro.h>
#include <functional.h>

namespace trt {
	/* Solves the equation of radiative transfer
	  * dI/dx = eta - chi * I
	  * along the "bound" beam getAbsEm from zmin until zmax,
	  * and is sure to sample at z=0 (if in range).
	  * uses GSU, future integration methods
	  * will be defined in other functions/ */
	double integrate_eort(auto getAbsEm);
}

#endif
