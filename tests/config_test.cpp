#include <iostream>
#include <config.h>
#include <stdexcept>
#include <fstream>
#include <sstream>

int main(int argv, char** argc) {
	trt::Config C;
	C.loadArgs(argv, argc);
	
	if( C.isset("FILE") ) {
		std::ifstream file(C.getString("FILE"));
		std::stringstream buffer;
		buffer << file.rdbuf();
		C.loadString(buffer.str());
	}
	
	if( C.isset("THREE") ) { // Extra testing mode 3:
		std::cout << "one:" << C.isset("one") << '\n'; // set 
		std::cout << "two:" << C.isset("two") << '\n'; // not set
		std::cout << "I1:" << C.getInt("I1") << '\n';
		try {
		int T =  C.getInt("I2");
		} catch (std::out_of_range X) {
			std::cout << "I2:" << X.what() << '\n';
		}
		std::cout << "D1:" << C.getDouble("D1") << '\n';
		try {
		double T =  C.getDouble("D2");
		} catch (std::invalid_argument X) {
			std::cout << "D2:" << X.what() << '\n';
		}
		try {
			double T = C.getBool("D3");
		} catch(std::out_of_range X) {
			std::cout << "D3:" << X.what() << '\n';
		}
		std::cout << "S1:" << C.getString("S1") << '\n';
		std::cout << "S2:" << C.getString("S2") << '\n';
		std::cout << "B1:" << C.getString("B1") << '\n';
		std::cout << "B2:" << C.getString("B2") << '\n';
		try {
			bool B = C.getBool("B3");
		} catch(std::invalid_argument X) {
			std::cout << "B3:" << X.what() << '\n';
		}
		void loadFile(std::string filename);
	}  else { // standard reply back 
		for(auto c : C) std::cout << c.first << ":" << c.second << std::endl;
	}
	return 0;
}
