#include <config.h>
#include <ctype.h> // isdigit()
#include <string>

namespace trt {

	void Config::loadArgs(int argc, char** argv) {
		std::string key = "";
		std::string param = "";
		for(int i = 1; i < argc; i++) {
			// Need to save previous arg as key
			if(!key.empty()) {
				// Case it has a parameter
				if(argv[i][0] != '-' || isdigit(argv[i][1])){
					param = argv[i];
				} else if (!key.empty()) { // Case it has no parameter
					param = "";
				}
				(*this)[key] = param;
				key.clear();
			}
			if (argv[i][0]=='-' && !isdigit(argv[i][1])){ // Identified new key
				key = argv[i];
			}
			// Case arg is neither parameter, nor key (does not start with -):
			// Then just ignore this argument.
		}
	}
}
