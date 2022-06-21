#include <iostream>
#include <config.h>

int main(int argv, char** argc) {
	trt::Config C;
	C.loadArgs(argv, argc);
	for(auto c : C) std::cout << c.first << ":" << c.second << std::endl;
	return 0;
}
