#ifndef LIBECTOOL_H
#define LIBECTOOL_H

#include <stdbool.h>

// Standard error codes
#define EC_ERR_INIT            -1
#define EC_ERR_READMEM         -2
#define EC_ERR_EC_COMMAND      -3
#define EC_ERR_INVALID_PARAM   -4
#define EC_ERR_UNSUPPORTED_VER   -5

#ifdef __cplusplus
extern "C" {
#endif

// Library init/release
int libectool_init();
void libectool_release();

// API functions to expose
int ec_is_on_ac(int *ac_present);

int ec_auto_fan_control();
int ec_set_fan_duty(int duty);
int ec_set_fan_rpm(int target_rpm, int fan_idx);
int ec_set_all_fan_rpm(int target_rpm);
int ec_get_fan_rpm(int *rpm, int fan_idx);
int ec_get_all_fan_rpm(int *rpms, int max_fans, int *num_fans_out);

int ec_get_max_temperature(float *max_temp);
int ec_get_max_non_battery_temperature(float *max_temp);

/* ASCII mode for printing, default off */
extern int ascii_mode;

#ifdef __cplusplus
}
#endif

#endif // LIBECTOOL_H
