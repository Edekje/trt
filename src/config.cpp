#include <config.h>
#include <ctype.h> // isdigit()
#include <string>
#include <iostream>

namespace trt {

	void Config::loadArgs(int argc, char** argv) {
		std::string key = "";
		std::string param = "";
		for(int i = 1; i < argc; i++) {
			// Need to save previous arg as key
			bool prev_is_key = !key.empty();
			bool current_is_key = argv[i][0] == '-'
							   && argv[i][1] != '\0' && !isdigit(argv[i][1]);
			//std::cout << i << ' ' << argv[i] << ' ' << prev_is_key << ' ' << current_is_key << '\n'; 
			if(current_is_key) {
				if(prev_is_key) (*this)[key] = "";
				key = argv[i]+1;
			} else {
				if(prev_is_key) (*this)[key] = argv[i];
				key = "";
			}
		}
			// Case arg is neither parameter, nor key (does not start with -):
			// Then just ignore this argument.
	}
}

