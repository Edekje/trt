#ifndef TRT_MICROPHYSICS_H
#define TRT_MICROPHYSICS_H

#include <config.h> // class Config
#include <hydro.h>	// class HydroVar

namespace trt {
	/* Absorption and Emission coefficients
	 * abs = 1/L, em = E/L/steradian  */
	class AbsEm {
		public:
		AbsEm() : abs{0}, em{0}, cutoff{false} {}
		AbsEm(double A, double E) : abs{A}, em{E}, cutoff{false} {}
		double abs, em;
		bool cutoff;
		/* cutoff=true indicates that quantity is below
		 * 'cutoff' value, and thus 'cutoff',
		 * that is, it will not be included in the integration.
		 * AbsEm values are still passed on so as to track changing
		 * quantities around shock. */
	};

	/* This class represents a function object for given power law p
	 * for function F_p(x) = x^{-(p-1)/2} \int_0^x dx' x'^{(p-3)/2} F(x'),
	 * where F(x) = x \int_x^{\infty} K_{5/3}(x') dx' is the synchrotron
	 * function. See eq. (6.31c) of Rybicki & Lightman (1979)
	 * and Fouka & Ouichaoui (2014). */
	class FP_Abstract {
		public:
		double p;
		// FP(double p); All derived classes should be initialised like this.
		virtual double operator()(double x) const = 0;
	};
	
	/* Implements function object for function F_p(x) from:
	 * "Analytical fits for the synchrotron emission from
	 * a power-law particle distribution with a sharp cutoff",
	 * Fouka, M, ; Ouichaoui, S. (MNRAS Aug. 2014)
	 * https://ui.adsabs.harvard.edu/abs/2014MNRAS.442..979F/abstract */
	class FP_Fouka : public FP_Abstract {
		double k_p, C_p;
		double a1, a2, a3, b1;
		public:
		FP_Fouka() : k_p{0} , C_p{0} {};
		FP_Fouka(double p);
		double operator()(double x) const;
	};
	
	class Microphysics {
		public:
		/* Initialises internal look-up data with given parameters
		 * for use later. Always requires at least the parameter
		 * nu_unit, the frequency unit used throughout these
		 * calculations, expressed in Hz: nu' = nu / n_unit. */
		//Microphysics(Config& param);
		/* Calculate fluid rest-frame absorption & emission
		 * coefficients from fluid variables. */
		virtual AbsEm getAbsEm(HydroVar HV, double nu) = 0; // should be virtual... not compiling
	};
	
	/* Canonical Synchrotron (CS) Microphysics model,
	 * described in docs/microphysics.md. */
	class CS_Microphysics : public Microphysics {
		double p, e_e, e_b, electron_fraction, M, L;
		FP_Fouka FP1, FP2;
		public:
		CS_Microphysics(Config& param);
		AbsEm getAbsEm(HydroVar HV, double nu);
	};
}

#endif
