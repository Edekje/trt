#include <hydro.h>
#include <microphysics.h>
#include <relativity.h>
#include <globals.h>

#include <fstream>
#include <algorithm>
#include <math.h>

#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkCellData.h>
#include <vtkPointData.h>

namespace trt {
	
	HydroVar::HydroVar() = default;
	HydroVar::HydroVar(double R, double TH) : rho{R}, e_th{TH} {}
	
	HydroVar1D::HydroVar1D() = default;
	HydroVar1D::HydroVar1D(double R, double TH, double U1) : HydroVar{R, TH}, u1{U1} {}
	
	/* read_Hydro1DSlice does what its name says.
	 * it may return duplicate points, we just bring these along to our interpolator.
	 */ 
	void read_Hydro1DSlice(std::string filename, double* &r, HydroVar1D* &HV, int &length, double &rmin, double &rmax) {
		// Initialise point reader
		vtkNew<vtkXMLUnstructuredGridReader> reader;
		reader->SetFileName(filename.c_str());
		reader->Update(); // This costs 10-30ms to call, using >99% of execution time in this function.
		vtkSmartPointer<vtkUnstructuredGrid> uG = reader->GetOutput();
		length = uG->GetNumberOfPoints()+1;
		vtkPointData* pd = uG->GetPointData();
		// Check presence of required data arrays
		std::string req_arrays[] = {"r", "rho", "u1", "p", "gammaeff"};
		for( std::string s : req_arrays )
			if( ! pd->HasArray(s.c_str()) )
				throw std::runtime_error( s + " array missing from " + filename+" - make sure Bmpi (point data) is enabled in amrvac.par!");
		// Locate upper and lower bounds of radius:
		double bounds[6];
		uG->GetBounds(bounds);
		rmin = bounds[0], rmax = bounds[1];
		// Allocate memory, including for boundary conditions on left & right
		r = new double[length];
		HV = new HydroVar1D[length];
		// Read points
		auto r_array      = pd->GetArray("r"),
			 rho_array    = pd->GetArray("rho"),
			 u1_array     = pd->GetArray("u1"),
			 p_array      = pd->GetArray("p"),
			 gammaeff_arr = pd->GetArray("gammaeff");

		for(int i = 1; i < length; i++) {
			r[i]		= r_array->GetComponent(i-1, 0);
			HV[i].rho	= rho_array->GetComponent(i-1, 0);
			HV[i].u1	= u1_array->GetComponent(i-1, 0);
			// Calculate e_th with pressure, rho & EOS:
			double p	 = p_array->GetComponent(i-1, 0);
			double gammaeff = gammaeff_arr->GetComponent(i-1,0);
			HV[i].e_th	= p / ( gammaeff - 1);
		}
		r[0]=-1e-100; // v. small negative value required so that interpolation does not break
		HV[0]=HV[1];  // center item has same prop as neighbour b.c.
		HV[0].u1 = 0; // in the center no motion
		// Check right-boundary are indeed valid:
		if(rmax > r[length-1])
			throw std::runtime_error("Maximum boundary value for r " + std::to_string(rmax) +
									 " greater than last value: " + std::to_string(r[length-2]) + ".");
	}

	HydroSim1D::HydroSim1D(std::string filename, int n_slices, double timestep, double t_0, int slice_start,
						   std::string timestepmode){
		this->t_0		= t_0;
		// timestep is not strictly necessary, as vtu files also carry timestamps in a field.	
		this->timestepmode = timestepmode;
		this->timestep	= timestep;
		if(timestepmode=="equal") {
			this->tmax      = t_0 + timestep * (n_slices-1);
		} else if(timestepmode=="log") {
			this->tmax  = t_0 * exp(timestep*(n_slices-1));
		} else {
			throw std::runtime_error("Invalid timestepmode: '"+timestepmode+"' - it must be either 'equal' or 'log'.");
		}
		this->n_slices	= n_slices;
		this->slice_len = new int[n_slices];
		this->r			= new double*[n_slices];
		this->slice		= new HydroVar1D*[n_slices];

		#pragma omp parallel for num_threads(trt::N_THREADS)
		for(int i=0; i < n_slices; i++) {
			constexpr int BF = 7; // supporting up to 999999 files, an obscene number.
			char filenumbercstr[BF];
			// Cast the filenumber to a c-string, 0 padded to at leat 4 characters,
			// with numbering starting at slice_start
			std::snprintf(filenumbercstr, BF, "%04i", i + slice_start);
			// construct individual filename:
			std::string temp = filename + filenumbercstr + ".vtu";
			// check if file exists before reading:
			std::fstream itemp;
			itemp.open(temp);
			if(! itemp ) throw std::runtime_error("File " + temp + " does not exist.");
			read_Hydro1DSlice(temp, r[i], slice[i], slice_len[i], rmin, rmax);
		}
	}

	HydroVar1D HydroSim1D::getHydroVar(Coordinate1D coord) {
		if(coord.r < 0 or coord.r > rmax) throw std::runtime_error("Radius r=" + std::to_string(coord.r) +
									 " requested by getHydroVar is out of range: (0<r<" + std::to_string(rmax) + ").");
		double fractional_slice;
		double t1, t2;
		int slice1, slice2;
		if(timestepmode=="equal") {
			fractional_slice = (coord.t_lab - t_0) / timestep;
			slice1 = floor( fractional_slice );
			slice2 = ceil( fractional_slice );
			t1 = slice1*timestep + t_0;
			t2 = slice2*timestep + t_0;
			if(slice1 < 0 || slice2 >= n_slices)
				throw std::runtime_error("T_lab=" + std::to_string(coord.t_lab) +
				" requested by getHydroVar is out of range: ("+std::to_string(t_0)
				+"<t<" + std::to_string(timestep*(n_slices-1)+t_0) + ").");
		} else if (timestepmode=="log") {
			if(coord.t_lab < t_0) 
				throw std::runtime_error("T_lab=" + std::to_string(coord.t_lab) +
				" requested by getHydroVar is out of range: ("+std::to_string(t_0)
				+"<t<" + std::to_string(exp(timestep*(n_slices-1))*t_0) + ").");
			fractional_slice = log(coord.t_lab / t_0) / timestep; // BUG WILL CRASH FOR NEGATIVE TIME.
			slice1 = floor( fractional_slice );
			slice2 = ceil( fractional_slice );
			t1 = exp(slice1*timestep)*t_0;
			t2 = exp(slice2*timestep)*t_0;
			if(slice1 < 0 || slice2 >= n_slices)
				throw std::runtime_error("T_lab=" + std::to_string(coord.t_lab) +
				" requested by getHydroVar is out of range: ("+std::to_string(t_0)
				+"<t<" + std::to_string(exp(timestep*(n_slices-1))*t_0) + ").");
		} else {
			throw std::runtime_error("Invalid timestepmode: '"+timestepmode+"' - it must be either 'equal' or 'log'.");
		}

		auto interp1d = [](double f1, double f2, double x1, double x2, double x) {
			if(x1 != x2)
				return (f2-f1)/(x2-x1)*(x-x1) + f1;
			else
				return f1;
		};
		
		int slice1rgreat = std::lower_bound(r[slice1], r[slice1] + slice_len[slice1], coord.r) - r[slice1];
		int slice1rless = slice1rgreat - 1;
		int slice2rgreat = std::lower_bound(r[slice2], r[slice2] + slice_len[slice2], coord.r) - r[slice2];
		int slice2rless = slice2rgreat - 1;

		HydroVar1D HV1L = slice[slice1][slice1rless];
		HydroVar1D HV1G = slice[slice1][slice1rgreat];
		HydroVar1D HV2L = slice[slice2][slice2rless];
		HydroVar1D HV2G = slice[slice2][slice2rgreat];

		auto interp2d = [&interp1d](double X1L, double X1R, double X2L, double X2R, double r1L, double r1R, double r2L, double r2R, double r, double t1, double t2, double t){
				double one = interp1d(X1L, X1R, r1L, r1R, r);
				double two = interp1d(X2L, X2R, r2L, r2R, r);
				//std::cout << "r: " << r1L << ' ' << r1R << ' ' << r2L << ' ' << r2R << " " << r << std::endl;
				//std::cout << one << ' ' << two << std::endl;
				return interp1d(one, two, t1, t2, t);
		};


		HydroVar1D returnme(
		interp2d(HV1L.rho, HV1G.rho, HV2L.rho, HV2G.rho, r[slice1][slice1rless], r[slice1][slice1rgreat], r[slice2][slice2rless], r[slice2][slice2rgreat], coord.r, t1, t2, coord.t_lab),
		interp2d(HV1L.e_th, HV1G.e_th, HV2L.e_th, HV2G.e_th, r[slice1][slice1rless], r[slice1][slice1rgreat], r[slice2][slice2rless], r[slice2][slice2rgreat], coord.r, t1, t2, coord.t_lab),
		interp2d(HV1L.u1, HV1G.u1, HV2L.u1, HV2G.u1, r[slice1][slice1rless], r[slice1][slice1rgreat], r[slice2][slice2rless], r[slice2][slice2rgreat], coord.r, t1, t2, coord.t_lab));

		return returnme;
	}	

}

