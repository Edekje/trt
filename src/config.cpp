#include <config.h>
#include <ctype.h> // isdigit()
#include <string> // string and stoi/stod
#include <sstream> // string
#include <stdexcept>

namespace trt {

	void Config::loadArgs(int argc, char** argv) {
		std::string key = "";
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
			// Case arg is neither parameter, nor key (does not start with -):
			// Then just ignore this argument.
		}
		if(!key.empty()) (*this)[key] = ""; // Don't forget the last key.

	}
	
	void Config::loadString(std::string filestring) {
		std::stringstream ss1(filestring);
		std::string line, key, param;
		while( std::getline(ss1, line).good() ) {
			std::stringstream ss2(line);
			ss2 >> key;
			if(ss2.fail() || key[0]!='-' || key.size()  < 2) continue;
			std::getline(ss2 >> std::ws, param); // Read till end of line & left trim whitespace
			if(ss2.fail()) param = "";
			param.erase(param.find_last_not_of(" \n\r\t")+1); // Right trim whitespace.
			(*this)[key.substr(1)] = param;
		}
	}

	bool Config::isset(std::string key) {
		return (*this).find(key)!=(*this).end();
	}

	int Config::getInt(std::string key) {
		auto it = (*this).find(key);
		if( it==(*this).end() ) throw std::out_of_range("getInt: Key \"-"+key+"\" does not exist.");
		try {
			return std::stoi(it->second); // Apply conversion
		} catch( std::invalid_argument &e ) {
			throw std::invalid_argument("getInt: Key \"-"+key+"\" represents an invalid integer format.");
		}
	}
	
	double Config::getDouble(std::string key) {
		auto it = (*this).find(key);
		if( it==(*this).end() ) throw std::out_of_range("getDouble: Key \"-"+key+"\" does not exist.");
		try {
			return std::stod(it->second); // Apply conversion
		} catch( std::invalid_argument &e ) {
			throw std::invalid_argument("getDouble: Key \"-"+key+"\" represents an invalid floating-point number format.");
		}
	}

	std::string Config::getString(std::string key) {
		auto it = (*this).find(key);
		if( it==(*this).end() ) throw std::out_of_range("getString: Key \"-"+key+"\" does not exist.");
		return it->second;
	}

	bool Config::getBool(std::string key) {
		auto it = (*this).find(key);
		if( it==(*this).end() ) throw std::out_of_range("getBool: Key \"-"+key+"\" does not exist.");
		std::string val = it->second;
		if(val=="1") return true;
		else if(val=="0") return false;
		else throw std::invalid_argument("getBool: Key \""+key+"\" must be either 0 or 1.");
	}
	
}

