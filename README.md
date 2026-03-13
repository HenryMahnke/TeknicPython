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