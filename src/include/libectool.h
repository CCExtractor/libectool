#ifndef LIBECTOOL_H
#define LIBECTOOL_H

#include <stdbool.h>

// Standard error codes
#define EC_SUCCESS              0
#define EC_ERR_INIT            -1
#define EC_ERR_READMEM         -2
#define EC_ERR_EC_COMMAND      -3
#define EC_ERR_INVALID_PARAM   -4

#ifdef __cplusplus
extern "C" {
#endif

// Library init/release
int libectool_init();
void libectool_release();

// API functions to expose
int is_on_ac(int *ac_present);
int auto_fan_control();
int set_fan_duty(int duty);
int get_max_temperature(float *max_temp);
int get_max_non_battery_temperature(float *max_temp);

/* ASCII mode for printing, default off */
extern int ascii_mode;

#ifdef __cplusplus
}
#endif

#endif // LIBECTOOL_H
