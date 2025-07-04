#pragma once
#include <stdexcept>
#include <string>

class ECController {
public:
    bool is_on_ac();
    void auto_fan_control();
    void set_fan_duty(int duty);
    float get_max_temperature();
    float get_max_non_battery_temperature();

private:
    void handle_error(int code, const std::string &msg);
};
