This project is a lightweight Python wrapper for Teknic ClearPath SC motors.

It currently targets Windows. Adapting it to Linux should be fairly straightforward once the SDK and library paths are updated.

## Development workflow

This project uses `uv` together with `scikit-build-core`, so the pybind11 extension is built and installed into the virtual environment instead of being copied into the source tree.

Create the environment and build the package with:

`uv venv`

`uv sync`

That is enough to build and install `TeknicMotors` into `.venv`, along with the generated `TeknicMotors.pyi` stub and the required `sFoundation20.dll`.

You can then run Python against the installed package with:

`uv run python -c "import TeknicMotors; print(TeknicMotors)"`

or run the sample script with:

`uv run python python_tests/python_example.py`

you may find that when switching between docker container and windows even when you revenv and resync it may not rebuild the build directory because there is something cached that makes it think it doesn't ahve to rebuild it

the workaroudn to this is running the following:
uv sync --reinstall-package teknicmotors

you could also run the cmake job to generate the build folder so that compile commands gets generated (because all of this is just to get hints, and not linking errors in the C++)
even if you delete the .cache file in the project, this is not the same. UV maintains it's own cache, for instance on windows that is located in {USER}\AppData\Local\uv\cache
hence the necessity to rebuilt 

when running in docker: 
currently run 
docker compose build 
docker compose up 
attach in vscode 
once in container : TODO need to automate: 

cd into linux_sdk/sFoundation/sFoundation 
run make
this will create libsFoundation20.so


LD_LIBRARY_PATH=/workspace/.venv/lib/python3.13/site-packages uv run python python_tests/python_example.py

right now builds with this really sus command from chatgpt, this tells it where to find the .so.1 library (do more research into this later)




TODO figure out how to run the USB Driver


there is also the concept of 3 names for linux shared libraries 
the real file, the runtime soname, and the linker/development name 
for example, 
real file: libfoo.so.1.2.3 
soname: libfoo.so.1 
linker/dev: libfoo.so
todo make a symlink to clean it up