#include <iostream>
#include <config.h>

int main(int argv, char** argc) {
	std::cout << "Testing config." << std::endl;
	trt::Config C;
	C.loadArgs(argv, argc);
	for(auto c : C) std::cout << c.first << " : " << c.second << '\n';
	return 0;
}
