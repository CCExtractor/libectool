#ifndef LIBECTOOL_H
#define LIBECTOOL_H

#include <stdbool.h>

// Standard error codes
#define EC_ERR_INIT                 -1
#define EC_ERR_READMEM              -2
#define EC_ERR_EC_COMMAND           -3
#define EC_ERR_INVALID_PARAM        -4
#define EC_ERR_UNSUPPORTED_VER      -5
#define EC_ERR_INVALID_RESPONSE     -6
#define EC_ERR_SENSOR_UNAVAILABLE   -7

#ifdef __cplusplus
extern "C" {
#endif

struct ec_temp_info {
    char sensor_name[32];
    int sensor_type;
    int temp;
    int temp_fan_off;
    int temp_fan_max;
};

struct ec_charge_state_info {
    int ac;
    int chg_voltage;
    int chg_current;
    int chg_input_current;
    int batt_state_of_charge;
};

// Library init/release
int libectool_init();
void libectool_release();

// API functions to expose
int ec_hello();

int ec_is_on_ac(int *ac_present);
int ec_get_charge_state(struct ec_charge_state_info *info_out);

int ec_get_num_fans(int *val);
int ec_enable_fan_auto_ctrl(int fan_idx);
int ec_enable_all_fans_auto_ctrl();
int ec_set_fan_duty(int percent, int fan_idx);
int ec_set_all_fans_duty(int percent);
int ec_set_fan_rpm(int target_rpm, int fan_idx);
int ec_set_all_fans_rpm(int target_rpm);
int ec_get_fan_rpm(int *rpm, int fan_idx);
int ec_get_all_fans_rpm(int *rpms, int rpms_size, int *num_fans_out);

int ec_get_num_temp_sensors(int *val) ;
int ec_get_temp(int sensor_idx, int *temp_out);
int ec_get_all_temps(int *temps_out, int max_len, int *num_sensors_out);
int ec_get_max_temp(int *max_temp);
int ec_get_max_non_battery_temp(int *max_temp);
int ec_get_temp_info(int sensor_idx, struct ec_temp_info *info_out);

/* ASCII mode for printing, default off */
extern int ascii_mode;

#ifdef __cplusplus
}
#endif

#endif // LIBECTOOL_H
