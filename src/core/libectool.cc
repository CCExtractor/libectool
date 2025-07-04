#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libectool.h"
#include "battery.h"
#include "comm-host.h"
#include "comm-usb.h"
#include "chipset.h"
#include "compile_time_macros.h"
#include "crc.h"
#include "ec_panicinfo.h"
#include "ec_flash.h"
#include "ec_version.h"
#include "i2c.h"
#include "lightbar.h"
#include "lock/gec_lock.h"
#include "misc_util.h"
#include "panic.h"
#include "usb_pd.h"

#include "framework_oem_ec_commands.h"

#ifndef _WIN32
#include "cros_ec_dev.h"
#endif

#define USB_VID_GOOGLE 0x18d1
#define USB_PID_HAMMER 0x5022
#define GEC_LOCK_TIMEOUT_SECS 30 /* 30 secs */
#define interfaces COMM_ALL

int ascii_mode;
// -----------------------------------------------------------------------------
//  Helper functions
// -----------------------------------------------------------------------------

int libectool_init()
{
    char device_name[41] = CROS_EC_DEV_NAME;
    uint16_t vid = USB_VID_GOOGLE, pid = USB_PID_HAMMER;
    int i2c_bus = -1;
    /*
     * First try the preferred /dev interface (which has a built-in mutex).
     * If the COMM_DEV flag is excluded or comm_init_dev() fails,
     * then try alternative interfaces.
     */
    if (!(interfaces & COMM_DEV) || comm_init_dev(device_name)) {
        /* For non-USB alt interfaces, we need to acquire the GEC lock */
        if (!(interfaces & COMM_USB) &&
            acquire_gec_lock(GEC_LOCK_TIMEOUT_SECS) < 0) {
            return -1;
        }
        /* If the interface is set to USB, try that (no lock needed) */
        if (interfaces == COMM_USB) {
#ifndef _WIN32
            if (comm_init_usb(vid, pid)) {
                /* Release the lock if it was acquired */
                release_gec_lock();
                return -1;
            }
#endif
        } else if (comm_init_alt(interfaces, device_name, i2c_bus)) {
            release_gec_lock();
            return -1;
        }
    }

    /* Initialize ring buffers for sending/receiving EC commands */
    if (comm_init_buffer()) {
        release_gec_lock();
        return -1;
    }

    return 0;
}

void libectool_release()
{
    /* Release the GEC lock. (This is safe even if no lock was acquired.) */
    release_gec_lock();

#ifndef _WIN32
    /* If the interface in use was USB, perform additional cleanup */
    if (interfaces == COMM_USB)
        comm_usb_exit();
#endif
}

int read_mapped_temperature(int id)
{
    int rv;

    if (!read_mapped_mem8(EC_MEMMAP_THERMAL_VERSION)) {
        /*
         *  The temp_sensor_init() is not called, which implies no
         * temp sensor is defined.
         */
        rv = EC_TEMP_SENSOR_NOT_PRESENT;
    } else if (id < EC_TEMP_SENSOR_ENTRIES)
        rv = read_mapped_mem8(EC_MEMMAP_TEMP_SENSOR + id);
    else if (read_mapped_mem8(EC_MEMMAP_THERMAL_VERSION) >= 2)
        rv = read_mapped_mem8(EC_MEMMAP_TEMP_SENSOR_B + id -
                      EC_TEMP_SENSOR_ENTRIES);
    else {
        /* Sensor in second bank, but second bank isn't supported */
        rv = EC_TEMP_SENSOR_NOT_PRESENT;
    }
    return rv;
}

// -----------------------------------------------------------------------------
// Top-level endpoint functions
// -----------------------------------------------------------------------------

int is_on_ac(int *ac_present) {
    int ret;
    uint8_t flags;

    if (!ac_present)
        return EC_ERR_INVALID_PARAM;

    ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    ret = ec_readmem(EC_MEMMAP_BATT_FLAG, sizeof(flags), &flags);

    if (ret <= 0) {
        libectool_release();
        return EC_ERR_READMEM;
    }

    *ac_present = !!(flags & EC_BATT_FLAG_AC_PRESENT);
    libectool_release();
    return EC_SUCCESS;
}

int auto_fan_control() {
    int ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    ret = ec_command(EC_CMD_THERMAL_AUTO_FAN_CTRL, 0, NULL, 0, NULL, 0);

    libectool_release();
    if (ret < 0)
        return EC_ERR_EC_COMMAND;
    return EC_SUCCESS;
}

int set_fan_duty(int duty) {
    if (duty < 0 || duty > 100)
        return EC_ERR_INVALID_PARAM;

    int ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    struct ec_params_pwm_set_fan_duty_v0 p_v0;
    p_v0.percent = duty;
    ret = ec_command(EC_CMD_PWM_SET_FAN_DUTY, 0, &p_v0, sizeof(p_v0),
            NULL, 0);

    libectool_release();
    if (ret < 0)
        return EC_ERR_EC_COMMAND;
    return EC_SUCCESS
}

int get_max_temperature(float *max_temp) {
    if (!max_temp)
        return EC_ERR_INVALID_PARAM;

    int ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    float t = -1.0f;
    int mtemp, temp;
    int id;

    for (id = 0; id < EC_MAX_TEMP_SENSOR_ENTRIES; id++) {
        mtemp = read_mapped_temperature(id);
        switch (mtemp) {
            case EC_TEMP_SENSOR_NOT_PRESENT:
            case EC_TEMP_SENSOR_ERROR:
            case EC_TEMP_SENSOR_NOT_POWERED:
            case EC_TEMP_SENSOR_NOT_CALIBRATED:
            continue;
            default:
                temp = K_TO_C(mtemp + EC_TEMP_SENSOR_OFFSET);
                if (temp > t)
                    t = temp;
        }
    }

    libectool_release();

    if (t < 0)
        return EC_ERR_READMEM;
    *max_temp = t;
    return EC_SUCCESS;
}

int get_max_non_battery_temperature(float *max_temp)
{
    if (!max_temp)
        return EC_ERR_INVALID_PARAM;

    int ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    struct ec_params_temp_sensor_get_info p;
    struct ec_response_temp_sensor_get_info r;
    float t = -1.0f;
    int mtemp, temp;

    for (p.id = 0; p.id < EC_MAX_TEMP_SENSOR_ENTRIES; p.id++) {
        mtemp = read_mapped_temperature(p.id);
        if (mtemp < 0)
            continue;
        ret = ec_command(EC_CMD_TEMP_SENSOR_GET_INFO, 0, &p,
                sizeof(p), &r, sizeof(r));
        if (ret < 0)
            continue;

        if(strcmp(r.sensor_name, "Battery")){
            temp = K_TO_C(mtemp + EC_TEMP_SENSOR_OFFSET);
            if (temp > t)
                t = temp;
        }
    }

    libectool_release();

    if (t < 0)
        return EC_ERR_READMEM;
    *max_temp = t;
    return EC_SUCCESS;
}
