#include <hydro.h>
#include <config.h>
#include <iostream>
#include <string>
#include <chrono>

using namespace std;

int main(int argv, char** argc) {
	trt::Config C;
	C.loadArgs(argv, argc);
	std::string filename = C.getString("f");
	int n_slices = C.getInt("ns");
	int start_slice = C.getInt("ss");
	double start_time = C.getInt("ss");
	double timestep = ( C.isset("ts") ) ? C.getDouble("ts") : 1.0;
	double N = ( C.isset("N") ) ? C.getInt("N") : 1;
	trt::HydroSim1D X(filename, n_slices, timestep, start_time, start_slice);
	double t = C.getDouble("t"), r = C.getDouble("r");
	trt::Coordinate1D coord(t, r);

	try {
		trt::HydroVar1D XX;
		auto a = std::chrono::high_resolution_clock::now();
		for(int i = 0; i < N; i++)
			XX = X.getHydroVar(coord);
		auto b = std::chrono::high_resolution_clock::now();
		cout.precision(8);
		cout << "rho: " << XX.rho << "\ne_th: " << XX.e_th << "\nu1: " << XX.u1 << endl;

		if(N > 1) cerr << "\n" << N << " evals of getHydroVar() took " << std::chrono::duration_cast<std::chrono::milliseconds>(b-a).count() << " ms." << endl;
	} catch(const std::runtime_error& e) {
		std::cout << e.what() << std::endl;
	}
}
