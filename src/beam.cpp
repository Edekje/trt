#include <beam.h>
#include <hydro.h>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <string>

namespace trt {
	
	/* Observer time, spacing, impact parameter, max radius, simulation start and end times, max radius */
	Beam1D::Beam1D(double t_obs, double dt, double a, double t_labmin, double t_labmax, double rmax) {
		if(a>rmax) throw std::runtime_error("Impact parameter a = " + std::to_string(a) + " > rmax = " + std::to_string(rmax)+" in Beam1D::Beam1D().");
		this->t_obs = t_obs;
		this->dt = dt;
		this->a = a;
		this->rmax = rmax;
		n1 = std::ceil(std::fmax(t_labmin-t_obs,-std::sqrt(rmax*rmax-a*a))/dt);
		n2 = std::floor(std::fmin(t_labmax-t_obs, std::sqrt(rmax*rmax-a*a))/dt);
	}

	int Beam1D::size() {
		return n2-n1+1;
	}

	Coordinate1D Beam1D::operator()(int i){
		if( i < n1 or i > n2) throw std::runtime_error("Beam1D::operator() index " + std::to_string(i) + " is out of bounds.");
		return Coordinate1D(t_obs+i*dt, std::sqrt(a*a+std::pow(i*dt,2)));
	}
}
