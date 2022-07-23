#ifndef TRT_BEAM_H
#define TRT_BEAM_H

#include <hydro.h>

namespace trt {
	
	class Beam {
		/* represents a collection of points corresponding to a light bean*/
		public:
		double t_obs;
		double zmin, zmax; // Range of continuous beam
	};

	class Beam1D : public Beam {
		public:
		double a; // Impact parameter
		double dt; // Spacing between points in t_lab.
		double rmax; // Maximal radius
		int n1, n2; // Start index, final index, inclusive.

		/* Observer time, spacing, impact parameter */
		Beam1D(double t_obs, double dx, double a, double t_labmin, double t_labmax, double rmax);
		int size();
		Coordinate1D operator()(int i);
		Coordinate1D operator()(double z);
	};
}

#endif
