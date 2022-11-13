#include <microphysics.h>
#include <config.h>
#include <iostream>
#include <chrono>


double e_th_cold(double rho, double p) {return p*3/2;}

int main(int argv, char** argc) {
	trt::Config C;
	C.loadArgs(argv, argc);

	if(C.isset("SPEED")) {
		auto a = std::chrono::high_resolution_clock::now();
		for(int i = 0; i < 1000000; i++) trt::FP_Fouka FP(2);
		auto b = std::chrono::high_resolution_clock::now();
		trt::FP_Fouka FP(2);
		for(int i = 0; i < 1000000; i++) FP(i);
		auto c = std::chrono::high_resolution_clock::now();
		std::cout << "1e6 inits of FP_Fouka took " << std::chrono::duration_cast<std::chrono::milliseconds>(b-a).count() << " ms." << '\n';
		std::cout << "1e6 evals of FP_Fouka took " << std::chrono::duration_cast<std::chrono::milliseconds>(c-b).count() << " ms." << '\n';
		return 0;
	}
	
	if(C.isset("SPEED2")) {
		trt::HydroVar HQ(1.1,e_th_cold(1.1,1.1));
		auto a = std::chrono::high_resolution_clock::now();
		for(int i = 0; i < 1000000; i++) trt::CS_Microphysics FP(C);
		auto b = std::chrono::high_resolution_clock::now();
		trt::CS_Microphysics CSRM(C);
		for(int i = 0; i < 1000000; i++) CSRM.getAbsEm(HQ, 1.0e10);
		auto c = std::chrono::high_resolution_clock::now();
		std::cout << "1e6 inits of CS_Microphysics took " << std::chrono::duration_cast<std::chrono::milliseconds>(b-a).count() << " ms." << '\n';
		std::cout << "1e6 evals of CS_Microphysics took " << std::chrono::duration_cast<std::chrono::milliseconds>(c-b).count() << " ms." << '\n';
		return 0;
	}

	if(C.isset("e_b")) {
		trt::CS_Microphysics CSM(C);
		trt::HydroVar HV(C.getDouble("rho"), e_th_cold(C.getDouble("rho"), C.getDouble("pressure")));
		double nu = C.getDouble("nu");
		trt::AbsEm AE = CSM.getAbsEm(HV, nu);
		if(!C.isset("src"))
			std::cout << "Abs: " << AE.abs << " Em: " << AE.em << '\n';
		else
			std::cout << "Source function: " << AE.em/AE.abs << '\n';
		return 0;
	}

    if(!C.isset("Q")) {
	trt::AbsEm X {1.0, 2.0};
	trt::HydroVar Y {3.9, e_th_cold(3.9, 5.9)};
	std::cout << "Abs: " << X.abs << " Em: " << X.em << '\n';
	std::cout << "Rho: " << Y.rho << " E_th: " << Y.e_th << '\n';
	
	trt::FP_Fouka FP1(C.getDouble("p1"));
	trt::FP_Fouka FP2(C.getDouble("p2"));
	trt::FP_Fouka FP3(C.getDouble("p3"));
	trt::FP_Fouka FP4(C.getDouble("p4"));
	double X1 = C.getDouble("X1");
	double X2 = C.getDouble("X2");
	double X3 = C.getDouble("X3");
	double X4 = C.getDouble("X4");
	std::cout << "FP1(X1=" << X1 << ")\t" << FP1(X1) << '\n';
	std::cout << "FP2(X2=" << X2 << ")\t" << FP2(X2) << '\n';
	std::cout << "FP3(X3=" << X3 << ")\t" << FP3(X3) << '\n';
	std::cout << "FP4(X4=" << X4 << ")\t" << FP4(X4) << '\n';
	} else {
		trt::FP_Fouka FP1(C.getDouble("p1"));
		trt::FP_Fouka FP2(C.getDouble("p2"));
		trt::FP_Fouka FP3(C.getDouble("p3"));
		trt::FP_Fouka FP4(C.getDouble("p4"));
		double X1 = C.getDouble("X1");
		double X2 = C.getDouble("X2");
		double X3 = C.getDouble("X3");
		double X4 = C.getDouble("X4");
		std::cout << FP1(X1) << '\n';
		std::cout << FP2(X2) << '\n';
		std::cout << FP3(X3) << '\n';
		std::cout << FP4(X4) << '\n';
	}
}
