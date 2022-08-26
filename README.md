# `trt` Teiresias Radiative Transfer 1.0 (Beta)

## Directions for Use:
    1. Compile as described in `docs/SETUP.md`
    2. Copy file `examples/input.txt` and configure with required parameters, with -inputname referring to the location of your hydro simulation. Make sure you performed the BHAC hydro simulation with the setting convert_type = 'vtuBmpi' in &filelist in amrvac.par, to give point-based output.
    3. Possible modes are `-tobs`, with a fixed time grid, and `-tvar`, which requires input of observation time points.
    4. Run with `./main -f input.txt > <OUTPUT FILE NAME>`.
    5. The results of the relativistic radiative transfer simulation will have been written to the output file.
    6. Optionally, the `video.py` script may be used for visualisation: `python video.py <OUTPUT FILE NAME>.

## Done:
- Configuration arguments via command line or file.
- Efficient synchrotron microphyics from Fouka (2014).
- Hydrosimulation 1D datastructure module for input from BHAC.
- A beam datastructure.
- Boosting of emission and absorption coefficients.
- A set of beams datastructure.
- An output module
- A coherent mainfile:
    - (t\_obs, a) grid mode
    - Random points as input via std input.
- Pressure / rho cut-off value.

## Testing stage:
- A beam integrator.
- Test whether Synge EOS gives sensible results

## Under development:
- 1.5D and 2D Radiative Transfer.

## To-do:
 - More documentation.
