#include <beam.h>
#include <config.h>
#include <iostream>

using namespace std;

int main(int argv, char** argc) {
	trt::Config C;
	C.loadArgs(argv, argc);

	double t_obs = C.getDouble("tobs");
	double dx = C.getDouble("dx");
	double a = C.getDouble("a");
	double t_labmin = C.getDouble("tmin");
	double t_labmax = C.getDouble("tmax");
	double rmax = C.getDouble("rmax");
	try {
		trt::Beam1D X (t_obs, dx, a, t_labmin, t_labmax, rmax);
		
		// test index error:
		if(C.isset("n")) X(C.getInt("n"));
		
		cout.precision(3);
		for(int i = X.n1; i <= X.n2; i++) cout << std::fixed << X(i).t_lab << ' ' << X(i).r << std::endl;
		cout << X.size() << endl;
	} catch (const std::runtime_error &e){
		cout << e.what() << endl;
	}	
}
