# libectool  

libectool is a shared library extracted from ectool, providing programmatic access to Embedded Controller (EC) functionalities on ChromeOS and compatible devices.  

## Features  
- Exposes EC control functions via a shared library (`libectool.so`).  
- Supports fan control, battery management, temperature monitoring, and more.  
- Designed for integration into other applications.  

## Build Instructions  
```sh
mkdir build && cd build
cmake ..
cmake --build .
```
