# Requirements
`trt` has been designed to use [VTK](https://vtk.org/) v9.2.0, as well as the GNU Scientific Library, [GSL 2.7.1.](https://www.gnu.org/software/gsl/) , building `trt` requires both VTK and GSL.

`trt` has been developed for use in Linux, but should also be compileable in MS Windows & MacOS.

# Installing VTK on Linux:

- Follow the instructions in "Using a VTK release" under "Build VTK" from [this guide](https://gitlab.kitware.com/vtk/vtk/-/blob/master/Documentation/dev/getting_started_linux.md#using-a-vtk-release).
- Make sure to verify your install by following the instructions under ["Verification"](https://gitlab.kitware.com/vtk/vtk/-/blob/master/Documentation/dev/getting_started_linux.md#verification).
- Input the correct VTK directory into VTK\_DIR variable in the CMakeLists.txt file.

# Installing GSL on Linux:
- In your directory of choice, download the latest stable version of GSL:
```
wget https://ftp.gnu.org/gnu/gsl/gsl-latest.tar.gz
```
- Extract the tar file:
```
tar -xvf gsl-latest.tar.gz
```
- `cd` into the resulting directory and install with:
```
./configure && make && sudo make install
```

# Compile `trt`
In order to compile Teiresias Radiative Transfer, `cd build`, then `cmake ..`, then  `make main`. This will produce the project's main executable.
