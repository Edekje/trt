#include <hydro.h>
#include <microphysics.h>
#include <config.h>
#include <integrate.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

int main(int argv, char** argc) {
	// Load Args
	trt::Config C;
	C.loadArgs(argv, argc);

	// Parse Args
	std::string filename = C.getString("fin");
	int n_slices = C.getInt("ns");
	double dx    = C.getDouble("dx");

	int start_slice = C.getInt("ss");
	double start_time = C.getInt("ss");
	double timestep = ( C.isset("ts") ) ? C.getDouble("ts") : 1.0;
	// Load hydro simulation & define microphysics
	trt::HydroSim1D X(filename, n_slices, timestep, start_time, start_slice);
	trt::CS_Microphysics MP(C);

	double t_obs, a, nu;

	cout << "Starting simulation, loaded " << n_slices << " slices!" << endl;

	while(true) {
		// Get input t_obs, a, nu.
//		if(cin.peek() == EOF|| cin.peek() == '\n') break;
		cin >> t_obs;
//		if(cin.peek() == EOF|| cin.peek() == '\n') break;
		cin >> a;
		if(cin.peek() == EOF|| cin.peek() == '\n') break;
		cin >> nu;
		// Calculate
		trt::Beam1D B(t_obs, dx, a, start_time, start_time+timestep*(n_slices-1), X.rmax);
		auto BB = trt::BindBeam(&X, &B, &MP, nu);
		double I = trt::integrate_eort(BB, B.zmin, B.zmax, dx);
		// Output
		cout.precision(6);
		cout << I << endl;
	}
}
