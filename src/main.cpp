#include <hydro.h>
#include <microphysics.h>
#include <integrate.h>
#include <config.h>
#include <globals.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

#include <chrono>

// Global Radiative Transfer Configuration Object:
trt::Config RTC;

int main(int argv, char** argc) {
	// Load CLI Arguments into RTC:
	RTC.loadArgs(argv, argc);
	// Case -v // --version
	if( RTC.isset("-version") || RTC.isset("v")) {
		std::cout << trt::TRT_VERSION << std::endl;
		return 0;
	}
	// Case -h /-- help
	if( RTC.isset("h") || RTC.isset("-help") || RTC.size() == 0 ) {
		std::cout << trt::TRT_VERSION << "\n\n" << trt::TRT_HELP << std::endl;
		return 0;
	}
	// Case -f: argument file given
	if( RTC.isset("f") ) {
		// Progress report
		std::cerr << "Loading arguments from: \"" << RTC.getString("f") << "\"..." << std::endl;
		
		std::ifstream file(RTC.getString("f"));
		std::stringstream buffer;
		buffer << file.rdbuf();
		RTC.loadString(buffer.str());
		// Finally, reload CLI arguments so that these have precedence over the file.
		RTC.loadArgs(argv, argc);
	}
	// Select radiative transfer mode: grid or point-input:
	if( RTC.isset("m") ) {
		// Threads
		// int n_threads		= (RTC.isset("n_threads")) ? RTC.getInt("n_threads") : 1;
		
		// Frequency
		double frequency = RTC.getDouble("frequency");

		// Microphysics
		trt::CS_Microphysics MP(RTC);
		
		// Integrator settings
		std::string integrator	= RTC.getString("integrator");
		double dz_max = RTC.getDouble("dz_max");
		double cutoff			= ( RTC.isset("cutoff") ) ? RTC.getDouble("cutoff") : 0.0;
		double precision, dz_min, variation_threshold;	
		if(integrator=="gsl") {
			precision		= RTC.getDouble("precision");
		} else if(integrator=="anavg") {
			dz_min			= RTC.getDouble("dz_min");
			variation_threshold = RTC.getDouble("variation_threshold");
		} else {
			throw std::invalid_argument("invalid integration mode: "+integrator);
		}

		// Default mode grid:
		double tobs_start = RTC.getDouble("tobs_start");
		double tobs_stop  = RTC.getDouble("tobs_stop");
		double tobs_step  = RTC.getDouble("tobs_step");
		double a_start    = RTC.getDouble("a_start");
		double a_stop     = RTC.getDouble("a_stop");
		double a_step     = RTC.getDouble("a_step");
		
		// Load slices
		std::string inputname	= RTC.getString("inputname");
		int slice_start_num		= RTC.getInt("slice_start_num");
		int slice_stop_num		= RTC.getInt("slice_stop_num");
		double slice_start_time	= RTC.getInt("slice_start_time");
		double slice_timestep	= RTC.getDouble("slice_timestep");
		auto timer_begin = std::chrono::high_resolution_clock::now();
		
		std::cerr << "Loading slices..." << std::endl;
		trt::HydroSim1D HS(inputname, slice_stop_num-slice_start_num,
						   slice_timestep, slice_start_time, slice_start_num);
		auto timer_end = std::chrono::high_resolution_clock::now();
		std::cerr << "Loaded " << slice_stop_num-slice_start_num << " slices in " << std::chrono::duration_cast<std::chrono::milliseconds>(timer_end-timer_begin).count()*0.001 << "s." << std::endl;
		
		// Force max_radius of beams to lie within limits
		double max_radius		= ( RTC.isset("maxradius") ) ? RTC.getDouble("maxradius") : HS.rmax;
		max_radius				= fmin(max_radius, HS.rmax);
		
		// Progress report
		std::cerr << "Starting computation..." << std::endl;

		// Output description:
		std::cout	<< "# teiresias radiative transfer output\n"
					<< "# inputname: " << inputname << "\n"
					<< "# slice_start_num, slice_stop_num, slice_start_time, slice_timestep, max_radius\n"
					<< slice_start_num << ", " << slice_stop_num << ", " << slice_start_time << ", " << slice_timestep << ", " << max_radius << "\n";
		std::cout << "# mode: grid" << std::endl;
		std::cout << "# tobs_start, tobs_stop, tobs_step, a_start, a_stop, a_step\n"
					<< tobs_start << ", " << tobs_stop << ", " << tobs_step << ", " << a_start << ", " << a_stop << ", " << a_step << "\n";
		std::cout << "# t_obs, a, frequency (Hz), optical depth, I (erg cm^-2 s^-1 Hz^-1)" << std::endl;
		std::cout.precision(6);
		
		timer_begin = std::chrono::high_resolution_clock::now();
		int beam_count = 0;
		


		//#pragma omp parallel for threads(n_threads)
		for(double tobs = tobs_start; tobs < tobs_stop; tobs += tobs_step) {
			for(double a = a_start; a < a_stop; a += a_step) {
				beam_count++;
				trt::Beam1D B(tobs, dz_min, a, slice_start_time,
							  slice_start_time+slice_timestep*(slice_stop_num-slice_start_num-1), max_radius);
				auto BB = trt::BindBeam(&HS, &B, &MP, frequency, cutoff);
				
				trt::AbsEm res;

				if(integrator=="gsl") {
					res.em = trt::integrate_eort(BB, B.zmin, B.zmax, dz_max, 0, precision);
				} else if(integrator=="anavg") {
					res = trt::integrate_eort_analytic(BB, B.zmin, B.zmax, dz_min, dz_max,
							 variation_threshold, trt::step_avg_eort, 0);
				}

				std::cout << tobs << ", " << a << ", " << frequency << ", " << res.abs << ", " << res.em << std::endl; 
			}
		}
		
		timer_end = std::chrono::high_resolution_clock::now();
		std::cerr.precision(4);
		std::cerr << "Computation completed. Integrated " << beam_count << " beams in " << std::chrono::duration_cast<std::chrono::milliseconds>(timer_end-timer_begin).count()*0.001 << "s." << std::endl;
	}

	return 0;
}
