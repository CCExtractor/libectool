cmake_minimum_required(VERSION 3.15...3.27)

project(
  ${SKBUILD_PROJECT_NAME}
  VERSION ${SKBUILD_PROJECT_VERSION}
  LANGUAGES CXX)

# Required for building Python extension modules via pybind11
find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)

# Find pybind11 (installed via pip/conda or system-wide)
find_package(pybind11 CONFIG REQUIRED)

if(NOT WIN32)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(libusb REQUIRED libusb-1.0)
  pkg_check_modules(libftdi1 REQUIRED libftdi1)
else()

endif()

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src/core)
add_subdirectory(src/bindings)
add_subdirectory(src/extern)

if(WIN32)
  add_subdirectory(src/getopt)
endif()

install(
  TARGETS ectool libectool libectool_py
  RUNTIME DESTINATION pyectool/bin # ectool CLI binary
  LIBRARY DESTINATION pyectool # libectool_py.so (shared Python module)
  ARCHIVE DESTINATION pyectool/lib # libectool.a (static lib)
)

install(
  DIRECTORY src/include/
  DESTINATION pyectool/include
  FILES_MATCHING
  PATTERN "libectool.h")
