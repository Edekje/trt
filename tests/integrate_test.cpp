#include <hydro.h>
#include <microphysics.h>
#include <config.h>
#include <integrate.h>

#include <iostream>
#include <string>

using namespace std;

int main(int argv, char** argc) {
	trt::Config C;
	C.loadArgs(argv, argc);
	double nu    = C.getDouble("nu");
	double a     = C.getDouble("a");
	double t_obs = C.getDouble("tobs");
	double z     = C.getDouble("z");
	double dx    = C.getDouble("dx");
	
	std::string filename = C.getString("fin");
	int n_slices = C.getInt("ns");
	int start_slice = C.getInt("ss");
	double start_time = C.getInt("ss");
	double timestep = ( C.isset("ts") ) ? C.getDouble("ts") : 1.0;
	trt::HydroSim1D X(filename, n_slices, timestep, start_time, start_slice);

	trt::Beam1D B(t_obs, dx, a, start_time, start_time+timestep*(n_slices-1), X.rmax);
	
	cout << B.zmin << ' ' << B.zmax << std::endl;
	
	trt::CS_Microphysics MP(C);

	auto BB = trt::BindBeam(&X, &B, &MP, nu);

	cout << BB(z).abs << ' ' << BB(z).em << std::endl;
}
