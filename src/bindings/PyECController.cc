#include <pybind11/pybind11.h>
#include "ECController.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

PYBIND11_MODULE(libectool_py, m) {
    m.doc() = "Python bindings for ectool";

    py::class_<ECController>(m, "ECController")
        .def(py::init<>())
        .def("is_on_ac", &ECController::is_on_ac, "Check if on AC power")
        .def("auto_fan_control", &ECController::auto_fan_control, "Enable automatic fan control")
        .def("set_fan_duty", &ECController::set_fan_duty,
             "Set fan duty cycle (0-100)", py::arg("duty"))
        .def("get_max_temperature", &ECController::get_max_temperature,
             "Get max temperature")
        .def("get_max_non_battery_temperature",
             &ECController::get_max_non_battery_temperature,
             "Get max non-battery temperature");

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
