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
### Clone the repository

## Install system-wide
```sh
sudo pip install .
```
Or:

```bash
sudo env "PIP_BREAK_SYSTEM_PACKAGES=1" pip install .
```
(Required on modern distros like Ubuntu 24.04 due to PEP 668.)

### Test from outside the repo dir
After installing, **do not run Python from the `libectool/` directory**, since it contains a `pyectool/` folder that may shadow the installed package.

Instead, test from another location, e.g.:

```sh
cd ..
sudo python -c "import pyectool; print(pyectool.is_on_ac())"
```

## VENV INSTALLATION

If you **don’t** want to touch system Python:

### Create venv

```bash
python3 -m venv ~/.venv/pyectool
source ~/.venv/pyectool/bin/activate
```

### Install your package

Inside the venv:
```bash
pip install .
```
### Test from outside the repo dir
```bash
cd ..
sudo env "PATH=$PATH" python -c "import pyectool; print(pyectool.is_on_ac())" 
```

### Available Functions

| Function                                   | Description                                                                      |
| ------------------------------------------ | -------------------------------------------------------------------------------- |
| `auto_fan_control()`               | Enables automatic fan control by the EC.                             |
| `get_max_non_battery_temperature() -> float` | Returns the highest temperature (in °C) from all sensors except the battery.     |
| `get_max_temperature() -> float`             | Returns the highest temperature (in °C) from all EC sensors including battery.   |
| `is_on_ac() -> bool`                       | Checks whether the device is running on AC power.                                |
| `set_fan_duty(percent: int)`               | Sets the fan duty cycle manually (0–100%).                                       |