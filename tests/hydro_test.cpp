#include <hydro.h>
#include <config.h>
#include <iostream>
#include <string>

int main(int argv, char** argc) {
	trt::Config C;
	C.loadArgs(argv, argc);
	std::string filename = C.getString("f");
	int n_slices = C.getInt("ns");
	int start_slice = C.getInt("ss");
	trt::HydroSim1D X(filename, n_slices, 1.0, 0.0, start_slice);

	
}
