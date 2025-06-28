#include <pybind11/pybind11.h>
#include "libectool.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

PYBIND11_MODULE(libectool_py, m) {
    m.doc() = "Python bindings for ectool";

    // Optional: expose init/release explicitly
    m.def("init", &libectool_init, "Initialize libectool");
    m.def("release", &libectool_release, "Release libectool");

    // Expose API functions
    m.def("is_on_ac", &is_on_ac, "Check if on AC power");
    m.def("auto_fan_control", &auto_fan_control, "Enable automatic fan control");
    m.def("set_fan_duty", &set_fan_duty, py::arg("duty"), "Set fan duty cycle (0-100)");
    m.def("get_max_temperature", &get_max_temperature, "Get max temperature");
    m.def("get_max_non_battery_temperature", &get_max_non_battery_temperature, "Get max non-battery temperature");

    // Expose global variable ascii_mode
    m.attr("ascii_mode") = py::cast(&ascii_mode, py::return_value_policy::reference);

    #ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
    #else
        m.attr("__version__") = "dev";
    #endif
}
