#include <hydro.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkCellData.h>

#include <vtkPointData.h>
namespace trt {
	
	HydroVar::HydroVar(double R, double TH) : rho{R}, e_th{TH} {}

	HydroVar1D::HydroVar1D(double R, double TH, double U1) : rho{R}, e_th{TH}, u1{U1} {}
	
	Coordinate1D::Coordinate1D(double T, double R) : Coordinate{T}, r{R} {}
	
	HydroSim1D::HydroSim1D(std::string filename, int nslices, double timestep, double t_0){
		vtkNew<vtkXMLUnstructuredGridReader> reader;
		reader->SetFileName(filename.c_str());
		reader->Update();
		vtkSmartPointer<vtkUnstructuredGrid> uG = reader->GetOutput();
		int n = uG->GetNumberOfPoints();
    
		vtkPointData* pd = uG->GetPointData();
	}

}



	
