#include <relativity.h>
#include <microphysics.h>
#include <config.h>
#include <iostream>

using namespace std;

int main(int argv, char** argc){
	trt::Config C;
	C.loadArgs(argv, argc);
	double bg = C.getDouble("bg"), csth=C.getDouble("csth");
	double df = trt::doppler_factor(bg, csth);
	double abs = C.getDouble("abs"), em = C.getDouble("em");
	trt::AbsEm AE2 = trt::boostAbsEmToLab(trt::AbsEm(abs,em), df);

	cout << "df: " << df << endl;
	cout << "abs: " << AE2.abs << endl;
	cout << "em: " << AE2.em << endl;

	return 0;
}
