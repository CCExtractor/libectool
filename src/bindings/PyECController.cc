#include <pybind11/pybind11.h>
#include "ECController.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

PYBIND11_MODULE(libectool_py, m) {
    m.doc() = "Python bindings for ectool";

    // Expose API functions
    m.def("is_on_ac", &ECController::is_on_ac, "Check if on AC power");
    m.def("auto_fan_control", &ECController::auto_fan_control, "Enable automatic fan control");
    m.def("set_fan_duty", &ECController::set_fan_duty, py::arg("duty"), "Set fan duty cycle (0-100)");
    m.def("get_max_temperature", &ECController::get_max_temperature, "Get max temperature");
    m.def("get_max_non_battery_temperature", &ECController::get_max_non_battery_temperature, "Get max non-battery temperature");

    #ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
    #else
        m.attr("__version__") = "dev";
    #endif
}
