#include <globals.h>

namespace trt {
	/* NUMERICAL CONSTANTS IN CGS */
	const double C_LIGHT    = 2.99792458e10;
	const double Q_ELECTRON = 4.80320425e-10;
	const double M_ELECTRON = 9.1093837015e-28;
	const double M_PROTON   = 1.67262192369e-24;



	const char* TRT_VERSION =
	"TRT: Teiresias Radiative Transfer v0.01 - (c) Ethan van Woerkom (2022)";

	const char* TRT_HELP =
	" * Performs 1D/2D Relativistic Radiative Transfer on hydrodynamic  *\n"
	" * simulations from the Black Hole Accretion Code (O. Porth, 2017).*\n\n"
	
	"   Takes arguments of form \"-<key> <optional parameter to key>\".\n\n"

	"\t Arguments:  \t Description: \n"
	"\t -f <file>   \t Provide configuration file with additional\n"
	"\t             \t arguments.\n"
	"\t --version   \t Display TRT version.\n"
	"\t -h / --help \t Display this help message.\n\n"

	"\t Notes:\n"
	"\n"
	" * This software has been released under the GNU LGPL 3.0 license. *"; 
}
