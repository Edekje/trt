#ifndef TRT_HYDRO_H
#define TRT_HYDRO_H

#include <string>

namespace trt {
	/* Stores rest frame fluid rest-mass density and rest-frame thermal energy density:
	 * thermal_energy = rest-frame_energy_density - rho = pressure / (GAMMA - 1)
	 * where GAMMA is the adiabatic index: GAMMA = 5/3 (non rel) 4/3 (rel)
	 * thus k = GAMMA - 1 for p = k * (e-rho) in Uhm, Z. (2011) */
	
	class HydroVar {
		public:
			HydroVar(double R, double TH);
			double rho, e_th;
	};
	
	class HydroVar1D {
		public:
			HydroVar1D(double R, double TH, double U1);
			double rho, e_th, u1;
	};

	class Coordinate {
		public:
			double t_lab;
	};

	class Coordinate1D :  public Coordinate {
		public: 
			Coordinate1D(double T, double R);
			double r;
	};
	
	/* Stores internal representation of a 1D hydrodynamic simulation
	 * read in .vtu format. filename gives the name and location of the 1st sim file.
	 * e.g. sims/data0001.vtu. nslices gives the number of such files 'slices':
	 * there will be files up to and including sims/data<nslices>.vtu, with appropriate 0 padding. */
	class HydroSim1D {
		double **r;
		HydroVar1D **slice; // Array of pointers to spatial slices
		int slice_len; // Array of lengths of spatial slices
		public:
			HydroSim1D(std::string filename, int nslices, double timestep, double t_0=0);
			HydroVar1D getHydroVar(Coordinate1D coord);
	};
	
}

#endif
