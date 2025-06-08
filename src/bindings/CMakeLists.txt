# Create the Python module
python_add_library(libectool_py MODULE libectool_py.cc WITH_SOABI)

# Link against required libraries
target_link_libraries(libectool_py PRIVATE pybind11::headers libectool)
target_include_directories(libectool_py PUBLIC ../include)
target_compile_definitions(libectool_py PUBLIC VERSION_INFO=${PROJECT_VERSION})
