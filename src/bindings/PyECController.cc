#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "ECController.h"
#include "libectool.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

py::dict temp_info_to_dict(const ec_temp_info& info) {
    py::dict d;
    d["sensor_name"] = std::string(info.sensor_name);
    d["sensor_type"] = info.sensor_type;
    d["temp"] = info.temp;
    d["temp_fan_off"] = info.temp_fan_off;
    d["temp_fan_max"] = info.temp_fan_max;
    return d;
}


PYBIND11_MODULE(libectool_py, m) {
    m.doc() = "Python bindings for ectool";

    py::class_<ECController>(m, "ECController")
        .def(py::init<>())
        .def("is_on_ac", &ECController::is_on_ac, "Check if on AC power")

        .def("get_num_fans", &ECController::get_num_fans,
        "Get number of fans")

        .def("enable_fan_auto_ctrl",
             &ECController::enable_fan_auto_ctrl,
             "Enable auto control for a fan",
             py::arg("fan_idx"))

        .def("enable_all_fans_auto_ctrl",
             &ECController::enable_all_fans_auto_ctrl,
             "Enable auto control for all fans")

        .def("set_fan_duty",
             &ECController::set_fan_duty,
             "Set fan duty cycle (0-100)",
             py::arg("percent"), py::arg("fan_idx"))

        .def("set_all_fans_duty",
             &ECController::set_all_fans_duty,
             "Set all fans duty cycle (0-100)",
             py::arg("percent"))

        .def("set_fan_rpm",
             &ECController::set_fan_rpm,
             "Set fan RPM",
             py::arg("target_rpm"), py::arg("fan_idx"))

        .def("set_all_fans_rpm",
             &ECController::set_all_fans_rpm,
             "Set all fans RPM",
             py::arg("target_rpm"))

        .def("get_fan_rpm",
             &ECController::get_fan_rpm,
             "Get single fan RPM",
             py::arg("fan_idx"))

        .def("get_all_fans_rpm",
            [](ECController &self) {
                return py::cast(self.get_all_fans_rpm());
            },
            "Get all fans RPM as list")

        .def("get_num_temp_sensors",
             &ECController::get_num_temp_sensors,
             "Get number of temperature sensors")

        .def("get_temp",
             &ECController::get_temp,
             "Get temperature in Celsius for one sensor",
             py::arg("sensor_idx"))

        .def("get_all_temps",
            [](ECController &self) {
                return py::cast(self.get_all_temps());
            },
            "Get all temperature values as list")

        .def("get_max_temp",
             &ECController::get_max_temp,
             "Get maximum temperature across all sensors")

        .def("get_max_non_battery_temp",
             &ECController::get_max_non_battery_temp,
             "Get maximum non-battery temperature")

        .def("get_temp_info",
            [](ECController &self, int sensor_idx) {
                ec_temp_info info = self.get_temp_info(sensor_idx);
                return temp_info_to_dict(info);
            },
            "Get detailed temperature info for a sensor",
            py::arg("sensor_idx"));

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
