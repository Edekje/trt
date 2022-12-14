#include <hydro.h>
#include <microphysics.h>
#include <integrate.h>
#include <config.h>
#include <globals.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <vector>

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
	if( (RTC.isset("tstep") || RTC.isset("tvar")) && (RTC.isset("astep") || RTC.isset("avar") ) ){
		// Load up times to observe at:
		std::vector<double> tobs;
		if(RTC.isset("tstep")) {
			double tobs_start = RTC.getDouble("tobs_start");
			double tobs_stop  = RTC.getDouble("tobs_stop");
			double tobs_step  = RTC.getDouble("tobs_step");
			for(double t = tobs_start; t < tobs_stop; t += tobs_step) {
				tobs.push_back(t);
			}
		} else { // RTC.isset("tvar")
			std::cerr << "In -tvar mode. Please input t_obs to iterate over (use -tgrid for a fixed tobs grid), end input with 'q':  " << std::endl;
			std::string t;  
			std::cin >> t; // get tobs
			// Read until there is no more input from cin
			while(t != "q") {
				tobs.push_back(std::stod(t));
				std::cin >> t; // get tobs
			}
		}
		if(tobs.size()==0) throw std::invalid_argument("No valid t_obs to iterate over.");
		
		std::vector<double> aobs;
		if(RTC.isset("astep")) {
			double aobs_start = RTC.getDouble("a_start");
			double aobs_stop  = RTC.getDouble("a_stop");
			double aobs_step  = RTC.getDouble("a_step");
			for(double a = aobs_start; a < aobs_stop; a += aobs_step) {
				aobs.push_back(a);
			}
		} else { // RTC.isset("avar")
			std::cerr << "In -avar mode. Please input impact parameter a to iterate over (use -agrid for a fixed a grid), end input with 'q':  " << std::endl;
			std::string a;  
			std::cin >> a; // get tobs
			// Read until there is no more input from cin
			while(a != "q") {
				aobs.push_back(std::stod(a));
				std::cin >> a; // get tobs
			}
		}
		if(aobs.size()==0) throw std::invalid_argument("No valid a to iterate over.");

		// Threads
		trt::N_THREADS = (RTC.isset("n_threads")) ? RTC.getInt("n_threads") : 1;
		std::cerr << "TRT running on " << trt::N_THREADS << " threads." << std::endl;
		
		// Get frequencies starting with frequency, frequency1, frequency2, frequency3...
		std::vector<double> frequencies;
		frequencies.push_back(RTC.getDouble("frequency"));
		for(int i = 1; RTC.isset("frequency"+std::to_string(i)); i++) {
			frequencies.push_back(RTC.getDouble("frequency"+std::to_string(i)));
		}

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

		// Load slices
		std::string inputname	= RTC.getString("inputname");
		int slice_start_num		= RTC.getInt("slice_start_num");
		int slice_stop_num		= RTC.getInt("slice_stop_num");
		double slice_start_time	= RTC.getDouble("slice_start_time");
		double slice_timestep	= RTC.getDouble("slice_timestep");
		std::string timestepmode= RTC.getString("slice_timestepmode");
		auto timer_begin = std::chrono::high_resolution_clock::now();
		
		std::cerr << "Loading slices..." << std::endl;
		// Melani (2004) relativistic equation of state:
		double GammaSynge = (RTC.isset("GammaSynge")) ? RTC.getDouble("GammaSynge") : 5.0/3.0;
		trt::HydroSim1D HS(inputname, slice_stop_num-slice_start_num,
						   slice_timestep, slice_start_time, slice_start_num, timestepmode, GammaSynge);
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
					<< "# slice_start_num, slice_stop_num, slice_start_time, slice_timestep, slice_timestepmode, max_radius, GammaSynge\n"
					<< slice_start_num << ", " << slice_stop_num << ", " << slice_start_time << ", " << slice_timestep << ", " << timestepmode << ", " << max_radius << ", " << GammaSynge << "\n";
		std::cout << "# cutoff, M, L, density_rescaling_factor, angle_average_param\n" << cutoff << ", " << RTC.getDouble("M") << ", " << RTC.getDouble("L") << ", " << MP.density_rescaling_factor << ", " << ( (RTC.isset("angle_average_param")) ? 0 : RTC.getInt("angle_average_param")) << std::endl;
		
		std::cout << "# frequencies\n" << frequencies[0];
		for(unsigned int i = 1; i < frequencies.size(); i++){
			std::cout << ", " << frequencies[i];
		} std::cout << std::endl;
		if(RTC.isset("tstep")) {
			std::cout << "# tobs_start, tobs_stop, tobs_step\n"
			<< RTC.getDouble("tobs_start") << ", " << RTC.getDouble("tobs_stop") << ", " << RTC.getDouble("tobs_step") << std::endl;
		} else { // RTC.isset("tvar")
			std::cout << "# tobs:\n";
			std::cout << tobs[0];
			for(unsigned int i = 1; i < tobs.size(); i++)
				std::cout << ", " << tobs[i];
			std::cout << std::endl;
		}
		if(RTC.isset("astep")) {
			std::cout << "# a_start, a_stop, a_step\n"
			<< RTC.getDouble("a_start") << ", " << RTC.getDouble("a_stop") << ", " << RTC.getDouble("a_step") << std::endl;
		} else { // RTC.isset("avar")
			std::cout << "# a:\n";
			std::cout << aobs[0];
			for(unsigned int i = 1; i < aobs.size(); i++)
				std::cout << ", " << aobs[i];
			std::cout << std::endl;
		}
		std::cout << "# t_obs, a, frequency (Hz), optical depth, I (erg cm^-2 s^-1 Hz^-1)" << std::endl;
		std::cout.precision(6);
		
		// Set up jobs queue
		struct beam_job {
			double tobs, a, frequency, tau, I;
		};
		std::vector<beam_job> beams;
		beam_job X;
		for(double t : tobs) {
			X.tobs = t;
			for(double a : aobs) {
				X.a = a;
				for(double freq : frequencies) {
					X.frequency = freq;
					beams.push_back(X);
				}
			}
		}
		
		// Perform jobs
		timer_begin = std::chrono::high_resolution_clock::now();
		#pragma omp parallel for num_threads(trt::N_THREADS)
		for(unsigned int i = 0; i < beams.size(); i++){
			trt::CS_Microphysics MP2(RTC);
			trt::Beam1D B(beams[i].tobs, dz_min, beams[i].a, slice_start_time,
						  HS.tmax, max_radius);
			auto BB = trt::BindBeam(&HS, &B, &MP2, beams[i].frequency, cutoff);
			
			trt::AbsEm res;
			if(integrator=="gsl") {
				res.em = trt::integrate_eort(BB, B.zmin, B.zmax, dz_max, 0, precision);
			} else if(integrator=="anavg") {
				res = trt::integrate_eort_analytic(BB, B.zmin, B.zmax, dz_min, dz_max,
						 variation_threshold, trt::step_avg_eort, 0);
			}
			beams[i].tau = res.abs;
			beams[i].I   = res.em;
		}
		// Output jobs
		for(unsigned int i = 0; i < beams.size(); i++){
			std::cout << beams[i].tobs << ", " << beams[i].a << ", " << beams[i].frequency
				      << ", " << beams[i].tau << ", " << beams[i].I << std::endl; 
		}
		// Summary
		timer_end = std::chrono::high_resolution_clock::now();
		std::cerr.precision(4);
		std::cerr << "Computation completed. Integrated " << beams.size() << " beams in "
			      << std::chrono::duration_cast<std::chrono::milliseconds>(timer_end-timer_begin).count()*0.001
				  << "s." << std::endl;
	}

	return 0;
}
