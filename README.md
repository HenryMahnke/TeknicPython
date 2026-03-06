This project is a simple lightweight python wrapper for Teknic Clearpath SC motors 

This project was to run on windows, and to adapt to Linux, it should be quite straightforward. The differences will be noted at the bottom of the readme 

when we make the python wrapper, we can then:
set "PYTHONPATH=<CURDIRECTORY>" && uv run pybind11-stubgen TeknicMotors

to generate the stubs from the .pyd file

then do note that to get method hints you will have to create a pyright config.json file that specifies python version, as well as extraPaths and stubPath