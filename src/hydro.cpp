#include <hydro.h>

namespace trt {
	
	HydroVar::HydroVar(double R, double TH) : rho{R}, e_th{TH} {}

	HydroVar1D::HydroVar1D(double R, double TH, double U1) : rho{R}, e_th{TH}, u1{U1} {}

}



	
