# Pyectool

**Pyectool** is a Python package with C++ bindings for interacting with the Embedded Controller (EC) on ChromeOS and Framework devices. It is extracted from and based on [`ectool`](https://gitlab.howett.net/DHowett/ectool) utility, and exposes EC control functions directly to Python programs via a native extension.

## Features

- Python bindings for EC functionality using `pybind11`.
- Supports fan duty control, temperature reading, AC power status, and more.
- Designed for integration with hardware management or fan control tools.
- Shared core logic with `libectool` for C/C++ integration.

---

## Build & Install (Python Package)

We use [`scikit-build-core`](https://scikit-build-core.readthedocs.io/en/latest/) to build the C++ extension via CMake.

### Prerequisites

Install the required system dependencies:

```sh
sudo apt update
sudo apt install -y libusb-1.0-0-dev libftdi1-dev pkg-config
````
### Clone the repository and switch to the Python package branch

```sh
git clone --branch dev_py_pkg_build https://github.com/AhmedYasserrr/libectool.git
cd libectool
```
### Install the package
```sh
pip install .
```

After installing, **do not run Python from the `libectool/` directory**, since it contains a `pyectool/` folder that may shadow the installed package.

Instead, test from another location, e.g.:

```sh
cd ..
python -c "import pyectool; print(pyectool.__version__)"
```
