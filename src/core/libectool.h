#ifndef LIBECTOOL_H
#define LIBECTOOL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Library init/release
int libectool_init();
void libectool_release();

// API functions to expose
bool is_on_ac();
void pause_fan_control();
void set_fan_speed(int speed);
float get_max_temperature();
float get_max_non_battery_temperature();

/* ASCII mode for printing, default off */
extern int ascii_mode;

#ifdef __cplusplus
}
#endif

#endif // LIBECTOOL_H
