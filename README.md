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