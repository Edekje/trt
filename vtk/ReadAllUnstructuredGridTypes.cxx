// See: https://kitware.github.io/vtk-examples/site/Cxx/IO/ReadAllUnstructuredGridTypes/
// And: https://kitware.github.io/vtk-examples/site/Cxx/IO/DumpXMLFile/
// For an example.

// reading the Unstructured grid:
#include <vtkUnstructuredGrid.h>
//#include <vtkUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridReader.h>

// Reading the cell data
#include <vtkCellData.h>
//#include <vtkCellTypes.h>

//#include <vtkCellData.h>
//#include <vtkCellTypes.h>
//#include <vtkDataSet.h>
//#include <vtkDataSetReader.h>
//#include <vtkFieldData.h>
//#include <vtkImageData.h>
//#include <vtkNew.h>
#include <vtkPointData.h>
//#include <vtkPolyData.h>
//#include <vtkRectilinearGrid.h>
//#include <vtkSmartPointer.h>
//#include <vtkStructuredGrid.h>
//#include <vtkUnstructuredGrid.h>
//#include <vtkXMLCompositeDataReader.h>
//#include <vtkXMLImageDataReader.h>
//#include <vtkXMLPolyDataReader.h>
//#include <vtkXMLReader.h>
//#include <vtkXMLRectilinearGridReader.h>
//#include <vtkXMLStructuredGridReader.h>
//#include <vtkXMLUnstructuredGridReader.h>
//#include <vtksys/SystemTools.hxx>

//vtk indent
#include <vtkIndent.h>


#include<iostream>
#include<string>


//using namespace std;

struct Densities {
	double* rho, *r;
	int n;
};

Densities read_points(char* filename) {
	vtkNew<vtkXMLUnstructuredGridReader> reader;
	reader->SetFileName(filename);
	reader->Update();
	vtkSmartPointer<vtkUnstructuredGrid> uG = reader->GetOutput();
	Densities ret;
	ret.n = uG->GetNumberOfPoints();
    
	vtkPointData* pd = uG->GetPointData();

	ret.rho = new double[ret.n];
    ret.r = new double[ret.n];

	for(int i = 0; i < ret.n; i++) {
		ret.rho[i] = pd->GetArray(0)->GetComponent(i, 0);
		ret.r[i]   = pd->GetArray(10)->GetComponent(i, 0);
	}

	return ret;
	
}

int main(int argv, char** argc){
	vtkNew<vtkXMLUnstructuredGridReader> reader;
	reader->SetFileName(argc[1]);
	reader->Update();
	vtkSmartPointer<vtkUnstructuredGrid> uG = reader->GetOutput();
	int numberOfCells = uG->GetNumberOfCells(), numberOfPoints = uG->GetNumberOfPoints();
	cout << argc[1] << " contains a " << uG->GetClassName() << endl;
	cout << argc[1] << "With " << numberOfCells << " cells " <<" and " << numberOfPoints << " points." << endl;
	
	// Counts # of occurrences of each cell type
	typedef std::map<int, int> CellContainer;
    CellContainer cellMap;
    for (int i = 0; i < numberOfCells; i++)
    {
      cellMap[uG->GetCellType(i)]++;
    }

	// Prints # of occ of each cell type

    CellContainer::const_iterator it = cellMap.begin();
    while (it != cellMap.end())
    {
      std::cout << "\tCell type "
                << vtkCellTypes::GetClassNameFromTypeId(it->first) << "(" << it->first << ") occurs "
                << it->second << " times." << std::endl;
      ++it;
    }

	// Now check for point data
    vtkPointData* pd = uG->GetPointData();
    if (pd)
    {
      std::cout << " contains point data with " << pd->GetNumberOfArrays()
                << " arrays." << std::endl;
      for (int i = 0; i < pd->GetNumberOfArrays(); i++)
      {
        std::cout << "\tArray " << i << " is named "
                  << (pd->GetArrayName(i) ? pd->GetArrayName(i) : "NULL")
                  << " has " << pd->GetArray(i)->GetNumberOfTuples()
                  << " tuples"
                  << " with " << pd->GetArray(i)->GetNumberOfComponents()
                  << " components"
                  << " of type " << pd->GetArray(i)->GetClassName()
                  << std::endl;
      }
    }

	 // Now check for cell data
    vtkCellData* cd = uG->GetCellData();
    if (cd)
    {
      std::cout << " contains cell data with " << cd->GetNumberOfArrays()
                << " arrays." << std::endl;
      for (int i = 0; i < cd->GetNumberOfArrays(); i++)
      {
        /*std::cout << "\tArray " << i << " is named "
                  << (cd->GetArrayName(i) ? cd->GetArrayName(i) : "NULL")
                  << std::endl;*/
		        std::cout << "\tArray " << i << " is named "
                  << (cd->GetArrayName(i) ? cd->GetArrayName(i) : "NULL")
                  << " has " << cd->GetArray(i)->GetNumberOfTuples()
                  << " tuples"
                  << " with " << cd->GetArray(i)->GetNumberOfComponents()
                  << " components"
                  << " of type " << cd->GetArray(i)->GetClassName()
                  << std::endl;
      }
    }

	// Now check for field data
    if (uG->GetFieldData())
    {
      std::cout << " contains field data with "
                << uG->GetFieldData()->GetNumberOfArrays() << " arrays."
                << std::endl;
      for (int i = 0; i < uG->GetFieldData()->GetNumberOfArrays(); i++)
      {
        std::cout
            << "\tArray " << i << " is named "
            << uG->GetFieldData()->GetArray(i)->GetName() << " has "
            << uG->GetFieldData()->GetArray(i)->GetNumberOfTuples()
            << " tuples"
            << " with "
            << uG->GetFieldData()->GetArray(i)->GetNumberOfComponents()
            << " components"
            << " of type "
            << uG->GetFieldData()->GetArray(i)->GetClassName()
            << std::endl;
      }
    }


	
	cout << "\nPrintSelf:" << endl;
	uG->PrintSelf(cout, vtkIndent(0));
	int array = std::stoi(argc[2]);
	int tuple = std::stoi(argc[3]);
	int comp = std::stoi(argc[4]);
	cout << "Array " << pd->GetArrayName(array) << " (" << array << ") at tuple " << tuple << ", component " << comp << ", has value: ";
	cout << pd->GetArray(array)->GetComponent(tuple,comp) << '.' << endl;
	
	cout << "Densities demo: " << endl;

	Densities K = read_points(argc[1]);

	for(int i = 0; i < K.n; i++) cout << "[" << i << "] R: " << K.r[i] << " rho: " << K.rho[i] << endl;

	cout << pd->GetArray("rho")->GetComponent(0,0) << endl;

	return 0;


}
