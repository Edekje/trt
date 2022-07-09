#include <hydro.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkCellData.h>
#include <fstream>

#include <vtkPointData.h>
namespace trt {
	
	HydroVar::HydroVar() = default;
	HydroVar::HydroVar(double R, double TH) : rho{R}, e_th{TH} {}
	
	HydroVar1D::HydroVar1D() = default;
	HydroVar1D::HydroVar1D(double R, double TH, double U1) : rho{R}, e_th{TH}, u1{U1} {}
	
	Coordinate1D::Coordinate1D(double T, double R) : Coordinate{T}, r{R} {}
	
	/* read_Hydro1DSlice does what its name says.
	 * it may return duplicate points, we just bring these along to our interpolator.
	 */ 
	void read_Hydro1DSlice(std::string filename, double* &r, HydroVar1D* &HV, int &length) {
		// Initialise point reader
		vtkNew<vtkXMLUnstructuredGridReader> reader;
		reader->SetFileName(filename.c_str());
		reader->Update();
		vtkSmartPointer<vtkUnstructuredGrid> uG = reader->GetOutput();
		length = uG->GetNumberOfPoints();
		vtkPointData* pd = uG->GetPointData();
		// Check presence of required data arrays
		std::string req_arrays[] = {"r", "rho", "u1", "p"};
		for( std::string s : req_arrays )
			if( ! pd->HasArray(s.c_str()) )
				throw std::runtime_error( s + " array missing from " + filename);
		// Locate upper and lower bounds of radius:
		double bounds[6];
		uG->GetBounds(bounds);
		double rmin = bounds[0], rmax = bounds[1];
		// Allocate memory, including for boundary conditions on left & right
		r = new double[length+2];
		HV = new HydroVar1D[length+2];
		// Read points
		for(int i = 1; i <= length; i++) {
			r[i]		= pd->GetArray("r")->GetComponent(i-1, 0);
			HV[i].rho	= pd->GetArray("rho")->GetComponent(i-1, 0);
			HV[i].u1	= pd->GetArray("u1")->GetComponent(i-1, 0);
			// Calculate e_th with pressure, rho & EOS:
			double p	 = pd->GetArray("p")->GetComponent(i-1, 0);
			HV[i].e_th	= p;
		}
		// Check left- and right-boundaries are indeed valid:
		if(rmin > r[1])
			throw std::runtime_error("Minimum boundary value for r " + std::to_string(rmin) +
									 " greater than first value: " + std::to_string(r[1]) + ".");
		if(rmax > r[length])
			throw std::runtime_error("Maximum boundary value for r " + std::to_string(rmax) +
									 " smaller than last value: " + std::to_string(r[length-1]) + ".");
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
			read_Hydro1DSlice(temp, r[i], slice[0], slice_len[0]);
		}
	}

}



	
