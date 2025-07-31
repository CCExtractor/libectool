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
sudo python -c "from pyectool import ECController; ec = ECController(); print(ec.is_on_ac())"
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
sudo env "PATH=$PATH" python -c "from pyectool import ECController; ec = ECController(); print(ec.is_on_ac())"
```

### Available Functions

All functions are methods of the `ECController` class. Instantiate it like so:

```python
from pyectool import ECController
ec = ECController()
```

Then use the methods as shown below:

| Method                                                  | Description                                                               |
| ------------------------------------------------------- | ------------------------------------------------------------------------- |
| `ec.is_on_ac() -> bool`                                 | Returns `True` if the system is on AC power, else `False`.                |
| `ec.get_num_fans() -> int`                              | Returns the number of fan devices detected.                               |
| `ec.enable_fan_auto_ctrl(fan_idx: int) -> None`         | Enables automatic fan control for a specific fan.                         |
| `ec.enable_all_fans_auto_ctrl() -> None`                | Enables automatic control for all fans.                                   |
| `ec.set_fan_duty(percent: int, fan_idx: int) -> None`   | Sets fan duty (speed) as a percentage for a specific fan.                 |
| `ec.set_all_fans_duty(percent: int) -> None`            | Sets the same duty percentage for all fans.                               |
| `ec.set_fan_rpm(target_rpm: int, fan_idx: int) -> None` | Sets a specific RPM target for a specific fan.                            |
| `ec.set_all_fans_rpm(target_rpm: int) -> None`          | Sets the same RPM target for all fans.                                    |
| `ec.get_fan_rpm(fan_idx: int) -> int`                   | Returns current RPM of a specific fan.                                    |
| `ec.get_all_fans_rpm() -> list[int]`                    | Returns a list of current RPM values for all fans.                        |
| `ec.get_num_temp_sensors() -> int`                      | Returns the total number of temperature sensors detected.                 |
| `ec.get_temp(sensor_idx: int) -> int`                   | Returns the temperature (in °C) for the given sensor index.               |
| `ec.get_all_temps() -> list[int]`                       | Returns a list of all sensor temperatures (in °C).                        |
| `ec.get_max_temp() -> int`                              | Returns the highest temperature across all sensors.                       |
| `ec.get_max_non_battery_temp() -> int`                  | Returns the highest temperature excluding battery-related sensors.        |
| `ec.get_temp_info(sensor_idx: int) -> ECTempInfo`       | Returns detailed info for a sensor, including name, type, and thresholds. |

---

### `ECTempInfo`

Returned by `get_temp_info()`, acts like a `dict` with:

* `sensor_name`: str
* `sensor_type`: int
* `temp`: int
* `temp_fan_off`: int
* `temp_fan_max`: int
