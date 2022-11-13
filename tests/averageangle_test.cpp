#include <microphysics.h>
#include <config.h>
#include <hydro.h>
#include <iostream>
#include <chrono>
#include <cmath>


double e_th_cold(double rho, double p) {return p*3/2;}


class CS_Microphysics_aa : public trt::CS_Microphysics {
	public:
	trt::AbsEm getAbsEm_angle(trt::HydroVar HV, double nu, double cos_pitch);
	//getAbsEm_aa(trt::HydroVar HV, double nu);
	using trt::CS_Microphysics::CS_Microphysics;
};

trt::AbsEm CS_Microphysics_aa::getAbsEm_angle(trt::HydroVar HV, double nu, double cos_pitch) {
	double e_b_old = e_b; // Backup e_b
	e_b = e_b * pow(cos_pitch,2); // Modify e_b: E_B -> E_B' = (cos alpha B)^2 = E_B cos^2
	trt::AbsEm returnme = getAbsEm(HV, nu);
	e_b = e_b_old;
	return returnme;
}

int main(int argv, char** argc) {
	trt::Config C;
	C.loadArgs(argv, argc);
	int k = C.getInt("k");
	int N = 1<<(k-1); // = 2^k
	
	std::cout << "angle (rad), cos(angle), em, abs" << std::endl;
	CS_Microphysics_aa MPA(C);
	trt::HydroVar HV(C.getDouble("r"), C.getDouble("e_th"));
	double nu = C.getDouble("nu");
	double avg_em  = 0;
	double avg_abs = 0;
	std::cout.precision(10);
	for(int i = 0; i <= N; i++) {
		double costheta = ((double) i) / ((double) N);
		double angle = acos(costheta);

		if(i==0)
			std::cout << angle << ' ' << costheta << ' ' << 0 << ' ' << 0 << std::endl;

		else {
			trt::AbsEm t = MPA.getAbsEm_angle(HV, nu, costheta);
			std::cout << angle << ' ' << costheta << ' ' << t.em << ' ' << t.abs << std::endl;
			avg_em  += t.em / (N+1);
			avg_abs += t.abs / (N+1);
		}
	}
	std::cout << "Avg:" << ' ' << avg_em << ' ' << avg_abs << std::endl;
	C.loadString("-angle_average_param "+std::to_string(k)+"\n");
	trt::CS_Microphysics MPB(C);
	trt::AbsEm r = MPB.getAbsEm(HV, nu);
	std::cout << "Internal aa:" << ' ' << r.em << ' ' << r.abs << std::endl;
}
