#include <hydro.h>
#include <config.h>
#include <iostream>
#include <string>

using namespace std;

int main(int argv, char** argc) {
	trt::Config C;
	C.loadArgs(argv, argc);
	std::string filename = C.getString("f");
	int n_slices = C.getInt("ns");
	int start_slice = C.getInt("ss");
	double start_time = C.getInt("ss");
	trt::HydroSim1D X(filename, n_slices, 1.0, start_time, start_slice);
	double t = C.getDouble("t"), r = C.getDouble("r");
	trt::Coordinate1D coord(t, r);
	try {
		trt::HydroVar1D XX = X.getHydroVar(coord);
		cout.precision(8);
		cout << "rho: " << XX.rho << "\ne_th: " << XX.e_th << "\nu1: " << XX.u1 << endl;
	} catch(const std::runtime_error& e) {
		std::cout << e.what() << std::endl;
	}
}
