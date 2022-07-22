#ifndef TRT_RELATIVITY_H
#define TRT_RELATIVITY_H

#include <microphysics.h>

namespace trt {
	/* Calculates doppler factor given beta*gamma and cos of angle
	 * theta between direction of motion and line-of-sight to obs. */
	double doppler_factor(double beta_gamma, double cos_theta);
	
	/* Applies appropriate boosts to chi_nu', eta_nu' to lab/obs frame
	 * values:
	 * chi_nu, eta_nu = boostAbsEmToLab(Rest_chi_eta(nu'=nu/df, df) */
	AbsEm boostAbsEmToLab(AbsEm InFluidRestFrame,
								 double doppler_factor);
}

#endif
