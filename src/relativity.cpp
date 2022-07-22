#ifndef TRT_RELATIVITY_H
#define TRT_RELATIVITY_H

#include <cmath>
#include <microphysics.h>

namespace trt {
	/* nu_obs = doppler_factor * nu_em, eq. (4.11) Rybicki & Lightman. */
	double doppler_factor(double beta_gamma, double cos_theta) {
		double gamma = std::sqrt( beta_gamma*beta_gamma + 1 );
		double beta = beta_gamma/gamma;
		double df = 1 / gamma / ( 1 - beta * cos_theta );
		return df;
	}
	
	/* chi_nu = chi_nu' / df ; eta_nu = eta_nu' * df^2 : 4.112/113 R&L/*/	
	AbsEm boostAbsEmToLab(AbsEm InFluidRestFrame, double doppler_factor) {
		return AbsEm(InFluidRestFrame.abs / doppler_factor,
					 InFluidRestFrame.em * (doppler_factor*doppler_factor));
	}
}

#endif
