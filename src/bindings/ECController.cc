#include "ECController.h"
#include "libectool.h"

void ECController::handle_error(int code, const std::string &msg) {
    if (code == EC_SUCCESS) return;

    std::string reason;
    switch (code) {
        case EC_ERR_INIT:           reason = "EC initialization failed"; break;
        case EC_ERR_READMEM:        reason = "EC memory read failed"; break;
        case EC_ERR_EC_COMMAND:     reason = "EC command failed"; break;
        case EC_ERR_INVALID_PARAM:  reason = "Invalid parameter"; break;
        default:                    reason = "Unknown error"; break;
    }

    throw std::runtime_error(msg + " (" + reason + ", code " + std::to_string(code) + ")");
}


bool ECController::is_on_ac() {
    int ac;
    int ret = is_on_ac(&ac);
    handle_error(ret, "Failed to read AC status");
    return ac;
}

void ECController::auto_fan_control() {
    int ret = auto_fan_control();
    handle_error(ret, "Failed to enable auto fan control");
}

void ECController::set_fan_duty(int duty) {
    int ret = set_fan_duty(duty);
    handle_error(ret, "Failed to set fan duty");
}

float ECController::get_max_temperature() {
    float t;
    int ret = get_max_temperature(&t);
    handle_error(ret, "Failed to get max temperature");
    return t;
}

float ECController::get_max_non_battery_temperature() {
    float t;
    int ret = get_max_non_battery_temperature(&t);
    handle_error(ret, "Failed to get non-battery temperature");
    return t;
}
