#include "ECController.h"
#include "libectool.h"

void ECController::handle_error(int code, const std::string &msg) {
    if (code == 0) return;

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

// -----------------------------------------------------------------------------
// Top-level Power Functions
// -----------------------------------------------------------------------------

bool ECController::is_on_ac() {
    int ac;
    int ret = ec_is_on_ac(&ac);
    handle_error(ret, "Failed to read AC status");
    return ac;
}

// -----------------------------------------------------------------------------
// Top-level fan control Functions
// -----------------------------------------------------------------------------

int ECController::get_num_fans() {
    int val = 0;
    int ret = ec_get_num_fans(&val);
    handle_error(ret, "Failed to get number of fans");
    return val;
}

void ECController::enable_fan_auto_ctrl(int fan_idx) {
    int ret = ec_enable_fan_auto_ctrl(fan_idx);
    handle_error(ret, "Failed to enable auto fan control");
}

void ECController::enable_all_fans_auto_ctrl() {
    int ret = ec_enable_all_fans_auto_ctrl();
    handle_error(ret, "Failed to enable auto control for all fans");
}

void ECController::set_fan_duty(int percent, int fan_idx) {
    int ret = ec_set_fan_duty(percent, fan_idx);
    handle_error(ret, "Failed to set fan duty");
}

void ECController::set_all_fans_duty(int percent) {
    int ret = ec_set_all_fans_duty(percent);
    handle_error(ret, "Failed to set duty for all fans");
}

void ECController::set_fan_rpm(int target_rpm, int fan_idx) {
    int ret = ec_set_fan_rpm(target_rpm, fan_idx);
    handle_error(ret, "Failed to set fan RPM");
}

void ECController::set_all_fans_rpm(int target_rpm) {
    int ret = ec_set_all_fans_rpm(target_rpm);
    handle_error(ret, "Failed to set RPM for all fans");
}

int ECController::get_fan_rpm(int fan_idx) {
    int rpm = 0;
    int ret = ec_get_fan_rpm(&rpm, fan_idx);
    handle_error(ret, "Failed to get fan RPM");
    return rpm;
}

std::vector<int> ECController::get_all_fans_rpm() {
    int num_fans = get_num_fans();
    std::vector<int> rpms(num_fans);
    int num_fans_out = 0;

    int ret = ec_get_all_fans_rpm(rpms.data(), num_fans, &num_fans_out);
    handle_error(ret, "Failed to get all fan RPMs");
    return rpms;
}

// -----------------------------------------------------------------------------
// Top-level temperature Functions
// -----------------------------------------------------------------------------
int ECController::get_num_temp_entries() {
    int val = 0;
    int ret = ec_get_num_temp_entries(&val);
    handle_error(ret, "Failed to get number of temp sensors");
    return val;
}

int ECController::get_temp(int sensor_idx) {
    int temp = 0;
    int ret = ec_get_temp(sensor_idx, &temp);
    handle_error(ret, "Failed to get temperature");
    return temp;
}

std::vector<int> ECController::get_all_temps() {
    int max_entries = get_num_temp_entries();
    std::vector<int> temps(max_entries);
    int num_sensors = 0;

    int ret = ec_get_all_temps(temps.data(), max_entries, &num_sensors);
    handle_error(ret, "Failed to get all temperatures");
    return temps;
}

int ECController::get_max_temp() {
    int temp = 0;
    int ret = ec_get_max_temp(&temp);
    handle_error(ret, "Failed to get max temperature");
    return temp;
}

int ECController::get_max_non_battery_temp() {
    int temp = 0;
    int ret = ec_get_max_non_battery_temp(&temp);
    handle_error(ret, "Failed to get max non-battery temperature");
    return temp;
}

ec_temp_info ECController::get_temp_info(int sensor_idx) {
    ec_temp_info info;
    int ret = ec_get_temp_info(sensor_idx, &info);
    handle_error(ret, "Failed to get temp sensor info");
    return info;
}
