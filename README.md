# Pyectool

**Pyectool** provides Python bindings for interacting with the Embedded Controller (EC) on ChromeOS and Framework devices.  
It is extracted from and based on [Dustin Howett's `ectool`](https://gitlab.howett.net/DHowett/ectool) and exposes EC control functions directly to Python via a native C++ extension built with `pybind11`.

Pyectool also provides a simple way to build the original `ectool` CLI tool, or to build `libectool`—a standalone C library that wrap most of ectool’s functionality, making it reusable in C/C++ projects or accessible from other languages. Both the CLI binary and the library are built automatically during installation.

## Features
- Python-native interface to low-level EC functionality via `pybind11`
- Supports fan duty control, temperature reading, AC power status, and more.
- Designed for hardware monitoring, thermal management, and fan control tooling.
- Bundles the native `ectool` CLI and `libectool` C library alongside the Python package:
  * `pyectool/bin/ectool`  (ectool CLI)
  * `pyectool/lib/libectool.a` (libectool static library)
  * `pyectool/include/libectool.h` (libectool C header)

---

## Installation

### Prerequisites

Install system dependencies:

```sh
sudo apt update
sudo apt install -y libusb-1.0-0-dev libftdi1-dev pkg-config
````
### Clone the repository

### Install the package

#### Option 1: System-wide (not recommended unless you know what you're doing)
```sh
sudo pip install .
```
Or:

```bash
sudo env "PIP_BREAK_SYSTEM_PACKAGES=1" pip install .
```
(Required on modern distros like Ubuntu 24.04 due to PEP 668.)

#### Option 2: Isolated virtual environment (recommended)
```bash
python3 -m venv ~/.venv/pyectool
source ~/.venv/pyectool/bin/activate
pip install .
```

### ⚠️ Important Note

After installation, **do not run Python from inside the `libectool/` directory**. It contains a `pyectool/` folder that may shadow the installed package.

Instead, test from a different directory:

```bash
cd ..
python -c "from pyectool import ECController; ec = ECController(); print(ec.is_on_ac())"
```

If you're using a virtual environment and want to preserve its `PATH`, use:
```bash
cd ..
sudo env "PATH=$PATH" python -c "from pyectool import ECController; ec = ECController(); print(ec.is_on_ac())"
```
This ensures the correct Python from your virtual environment is used even with `sudo`.

---

## Usage

### Create an EC controller instance

```python
from pyectool import ECController

ec = ECController()
```

### Available Methods


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

---

## License

BSD 3-Clause License
See the `LICENSE` file for full terms.
