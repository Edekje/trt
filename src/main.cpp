#include <iostream>
#include <config.h>
#include <fstream>
#include <sstream>
#include <globals.h>

// Global Radiative Transfer Configuration Object:
trt::Config RTC;

int main(int argv, char** argc) {
	// Load CLI & File Arguments into RTC:
	RTC.loadArgs(argv, argc);
	if( RTC.isset("-version")) {
		std::cout << TRT_VERSION << std::endl;
		return 0;
	}
	if( RTC.isset("-h") || RTC.isset("--help") || RTC.size() == 0 ) {
		std::cout << TRT_VERSION << "\n\n" << TRT_HELP << std::endl;
	}
	if( RTC.isset("f") ) {
		std::ifstream file(RTC.getString("f"));
		std::stringstream buffer;
		buffer << file.rdbuf();
		RTC.loadString(buffer.str());
		// Finally, reload CLI arguments so that these have precedence over the file.
		RTC.loadArgs(argv, argc);
	}
	// 

	return 0;
}
