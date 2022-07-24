#include <hydro.h>
#include <microphysics.h>
#include <relativity.h>

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
		reader->Update();
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
		for(int i = 1; i < length; i++) {
			r[i]		= pd->GetArray("r")->GetComponent(i-1, 0);
			HV[i].rho	= pd->GetArray("rho")->GetComponent(i-1, 0);
			HV[i].u1	= pd->GetArray("u1")->GetComponent(i-1, 0);
			// Calculate e_th with pressure, rho & EOS:
			double p	 = pd->GetArray("p")->GetComponent(i-1, 0);
			double gammaeff = pd->GetArray("gammaeff")->GetComponent(i-1,0);
			HV[i].e_th	= p / ( gammaeff - 1);
		}
		r[0]=0;
		HV[0]=HV[1];
		// Check right-boundary are indeed valid:
		if(rmax > r[length-1])
			throw std::runtime_error("Maximum boundary value for r " + std::to_string(rmax) +
									 " greater than last value: " + std::to_string(r[length-2]) + ".");
	}

	HydroSim1D::HydroSim1D(std::string filename, int n_slices, double timestep, double t_0, int slice_start){
		this->t_0		= t_0;
		// timestep is not strictly necessary, as vtu files also carry timestamps in a field.	
		this->timestep	= timestep;
		this->n_slices	= n_slices;
		this->slice_len = new int[n_slices];
		this->r			= new double*[n_slices];
		this->slice		= new HydroVar1D*[n_slices];

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
		if(coord.r > rmax) throw std::runtime_error("Radius r=" + std::to_string(coord.r) +
									 " requested by getHydroVar is out of range: (r<" + std::to_string(rmax) + ").");

		int slice1 = floor( (coord.t_lab - t_0) / timestep );
		int slice2 = ceil( (coord.t_lab - t_0) / timestep );
		if(slice1 < 0 || slice2 >= n_slices) throw std::runtime_error("T_lab=" + std::to_string(coord.t_lab) +
									 " requested by getHydroVar is out of range: ("+std::to_string(t_0)+"<t<" + std::to_string(timestep*(n_slices-1)+t_0) + ").");
		
		auto interp1d = [](double f1, double f2, double x1, double x2, double x) {
			return (f2-f1)/(x2-x1)*(x-x1) + f1;
		};
		
		double deltat = fmod(coord.t_lab-t_0, timestep);

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
				//std::cout << one << '_' << two << std::endl;
				return interp1d(one, two, t1, t2, t);
		};


		HydroVar1D returnme(
		interp2d(HV1L.rho, HV1G.rho, HV2L.rho, HV2G.rho, r[slice1][slice1rless], r[slice1][slice1rgreat], r[slice2][slice2rless], r[slice2][slice2rgreat], coord.r, 0, timestep, deltat),
		interp2d(HV1L.e_th, HV1G.e_th, HV2L.e_th, HV2G.e_th, r[slice1][slice1rless], r[slice1][slice1rgreat], r[slice2][slice2rless], r[slice2][slice2rgreat], coord.r, 0, timestep, deltat ),
		interp2d(HV1L.u1, HV1G.u1, HV2L.u1, HV2G.u1, r[slice1][slice1rless], r[slice1][slice1rgreat], r[slice2][slice2rless], r[slice2][slice2rgreat], coord.r, 0, timestep, deltat ));

		return returnme;
	}	

}

