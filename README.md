# libectool  

libectool is a shared library extracted from ectool, providing programmatic access to Embedded Controller (EC) functionalities on ChromeOS and compatible devices.  

## Features  
- Exposes EC control functions via a shared library (`libectool.so`).  
- Supports fan control, battery management, temperature monitoring, and more.  
- Designed for integration into other applications.  

## Build Instructions  
```sh
cd libectool
mkdir build && cd build
cmake ..
cmake --build .
```
##  Post Build Instructions
After building, you need to move `libectool.so` to a library directory where it can be found by your system:

### Option 1 — User-specific (Recommended for non-root users)
```sh
mkdir -p ~/.local/lib
cp src/libectool.so ~/.local/lib/libectool.so
export LD_LIBRARY_PATH="$HOME/.local/lib:$LD_LIBRARY_PATH"
```
To make it persistent across sessions, add the export to your shell configuration:
```sh
echo 'export LD_LIBRARY_PATH="$HOME/.local/lib:$LD_LIBRARY_PATH"' >> ~/.bashrc
```
### Option 2 — Global installation
```sh
sudo cp src/libectool.so /usr/local/lib/libectool.so
```
