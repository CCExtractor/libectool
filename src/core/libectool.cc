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
    int ret;
    uint8_t val;

    ret = ec_readmem(EC_MEMMAP_THERMAL_VERSION, sizeof(val), &val);
    if (ret <= 0 || val == 0)
        return EC_TEMP_SENSOR_NOT_PRESENT;

    if (id < EC_TEMP_SENSOR_ENTRIES) {
        ret = ec_readmem(EC_MEMMAP_TEMP_SENSOR + id, sizeof(val), &val);
        return (ret <= 0) ? EC_TEMP_SENSOR_ERROR : val;
    }

    // Check if second bank is supported
    if (val < 2)
        return EC_TEMP_SENSOR_NOT_PRESENT;

    ret = ec_readmem(
        EC_MEMMAP_TEMP_SENSOR_B + id - EC_TEMP_SENSOR_ENTRIES,
        sizeof(val), &val);
    return (ret <= 0) ? EC_TEMP_SENSOR_ERROR : val;
}

// -----------------------------------------------------------------------------
// Top-level General Functions
// -----------------------------------------------------------------------------
int ec_hello() {
    int ret;
    struct ec_params_hello p;
    struct ec_response_hello r;

    ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    p.in_data = 0xa0b0c0d0;

    ret = ec_command(EC_CMD_HELLO, 0,
                     &p, sizeof(p),
                     &r, sizeof(r));
    libectool_release();

    if (ret < 0)
        return EC_ERR_EC_COMMAND;

    if (r.out_data != 0xa1b2c3d4) {
        return EC_ERR_INVALID_RESPONSE;
    }

    return 0;
}

// -----------------------------------------------------------------------------
// Top-level Power Functions
// -----------------------------------------------------------------------------

int ec_is_on_ac(int *ac_present) {
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
    return 0;
}

// -----------------------------------------------------------------------------
// Top-level fan control Functions
// -----------------------------------------------------------------------------

int ec_get_num_fans(int *val) {
    int ret, fan_val, idx;
    struct ec_response_get_features r;

    if (!val)
        return EC_ERR_INVALID_PARAM;

    ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    ret = ec_command(EC_CMD_GET_FEATURES, 0, NULL, 0, &r, sizeof(r));
    if (ret >= 0 && !(r.flags[0] & BIT(EC_FEATURE_PWM_FAN)))
        *val = 0;

    for (idx = 0; idx < EC_FAN_SPEED_ENTRIES; idx++) {
        ret = ec_readmem(EC_MEMMAP_FAN + 2 * idx, sizeof(fan_val), &fan_val);

        if (ret <= 0)
            return EC_ERR_READMEM;

        if (fan_val == EC_FAN_SPEED_NOT_PRESENT)
            break;
    }

    *val = idx;
    libectool_release();
    return 0;
}

int ec_enable_fan_auto_ctrl(int fan_idx) {
    int ret, cmdver;
    int num_fans;
    struct ec_params_auto_fan_ctrl_v1 p_v1;

    ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    cmdver = 1;

    if (!ec_cmd_version_supported(EC_CMD_THERMAL_AUTO_FAN_CTRL, cmdver)) {
        libectool_release();
        return EC_ERR_UNSUPPORTED_VER;
    }

    ec_get_num_fans(&num_fans);

    if (fan_idx < 0 || fan_idx >= num_fans) {
        libectool_release();
        return EC_ERR_INVALID_PARAM;
    }

    p_v1.fan_idx = fan_idx;

    ret = ec_command(EC_CMD_THERMAL_AUTO_FAN_CTRL, cmdver,
                     &p_v1, sizeof(p_v1),
                     NULL, 0);
    libectool_release();

    return (ret < 0) ? EC_ERR_EC_COMMAND : 0;
}

int ec_enable_all_fans_auto_ctrl() {
    int ret;

    ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    ret = ec_command(EC_CMD_THERMAL_AUTO_FAN_CTRL, 0,
                     NULL, 0,
                     NULL, 0);
    libectool_release();

    return (ret < 0) ? EC_ERR_EC_COMMAND : 0;
}

int ec_set_fan_duty(int percent, int fan_idx) {
    int ret, cmdver;
    int num_fans;
    struct ec_params_pwm_set_fan_duty_v1 p_v1;

    if (percent < 0 || percent > 100)
        return EC_ERR_INVALID_PARAM;

    ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    ec_get_num_fans(&num_fans);
    if (fan_idx < 0 || fan_idx >= num_fans) {
        libectool_release();
        return EC_ERR_INVALID_PARAM;
    }

    cmdver = 1;

    if (!ec_cmd_version_supported(EC_CMD_PWM_SET_FAN_DUTY, cmdver)) {
        libectool_release();
        return EC_ERR_UNSUPPORTED_VER;
    }

    p_v1.fan_idx = fan_idx;
    p_v1.percent = percent;

    ret = ec_command(EC_CMD_PWM_SET_FAN_DUTY, cmdver,
                     &p_v1, sizeof(p_v1), NULL, 0);

    libectool_release();

    return (ret < 0) ? EC_ERR_EC_COMMAND : 0;
}

int ec_set_all_fans_duty(int percent) {
    int ret;
    struct ec_params_pwm_set_fan_duty_v0 p_v0;

    if (percent < 0 || percent > 100)
        return EC_ERR_INVALID_PARAM;

    ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    p_v0.percent = percent;

    ret = ec_command(EC_CMD_PWM_SET_FAN_DUTY, 0,
                     &p_v0, sizeof(p_v0), NULL, 0);

    libectool_release();

    return (ret < 0) ? EC_ERR_EC_COMMAND : 0;
}

int ec_set_fan_rpm(int target_rpm, int fan_idx) {
    int ret, cmdver;
    int num_fans;
    struct ec_params_pwm_set_fan_target_rpm_v1 p_v1;

    if (target_rpm < 0)
        return EC_ERR_INVALID_PARAM;

    ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    ec_get_num_fans(&num_fans);

    if (fan_idx < 0 || fan_idx >= num_fans) {
        libectool_release();
        return EC_ERR_INVALID_PARAM;
    }

    cmdver = 1;

    if (!ec_cmd_version_supported(EC_CMD_PWM_SET_FAN_TARGET_RPM, cmdver)) {
        libectool_release();
        return EC_ERR_UNSUPPORTED_VER;
    }

    p_v1.fan_idx = fan_idx;
    p_v1.rpm = target_rpm;

    ret = ec_command(EC_CMD_PWM_SET_FAN_TARGET_RPM, cmdver,
                     &p_v1, sizeof(p_v1), NULL, 0);
    libectool_release();

    return (ret < 0) ? EC_ERR_EC_COMMAND : 0;
}

int ec_set_all_fans_rpm(int target_rpm) {
    int ret;
    struct ec_params_pwm_set_fan_target_rpm_v0 p_v0;

    if (target_rpm < 0)
        return EC_ERR_INVALID_PARAM;

    ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    p_v0.rpm = target_rpm;

    ret = ec_command(EC_CMD_PWM_SET_FAN_TARGET_RPM, 0,
                     &p_v0, sizeof(p_v0), NULL, 0);

    libectool_release();
    return (ret < 0) ? EC_ERR_EC_COMMAND : 0;
}

int ec_get_fan_rpm(int *rpm, int fan_idx) {
    int ret, val, num_fans;

    if (!rpm)
        return EC_ERR_INVALID_PARAM;

    ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    ec_get_num_fans(&num_fans);

    if (fan_idx < 0 || fan_idx >= num_fans) {
        libectool_release();
        return EC_ERR_INVALID_PARAM;
    }

    ret = ec_readmem(EC_MEMMAP_FAN + 2 * fan_idx, sizeof(val), &val);
    if (ret <= 0)
        return EC_ERR_READMEM;

    switch (val) {
        case EC_FAN_SPEED_NOT_PRESENT:
            val = -1;
            break;
        case EC_FAN_SPEED_STALLED:
            val = -2;
            break;
    }

    libectool_release();

    *rpm = val;
    return 0;
}

int ec_get_all_fans_rpm(int *rpms, int rpms_size, int *num_fans_out) {
    int i, ret, val, num_fans;

    if (!rpms || !num_fans_out)
        return EC_ERR_INVALID_PARAM;

    ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    ec_get_num_fans(&num_fans);
    *num_fans_out = num_fans;

    for (i = 0; i < num_fans && i < rpms_size; i++) {
        ret = ec_readmem(EC_MEMMAP_FAN + 2 * i, sizeof(val), &val);
        if (ret <= 0)
            return EC_ERR_READMEM;

        switch (val) {
            case EC_FAN_SPEED_NOT_PRESENT:
                val = -1;
                break;
            case EC_FAN_SPEED_STALLED:
                val = -2;
                break;
        }
        rpms[i] = val;
    }

    libectool_release();
    return 0;
}

// -----------------------------------------------------------------------------
// Top-level temperature Functions
// -----------------------------------------------------------------------------

int ec_get_num_temp_entries(int *val) {
    if (!val)
        return EC_ERR_INVALID_PARAM;

    *val = EC_MAX_TEMP_SENSOR_ENTRIES;
    return 0;
}

int ec_get_temp(int sensor_idx, int *temp_out) {
    int mtemp, ret;

    if (!temp_out || sensor_idx < 0 || sensor_idx >= EC_MAX_TEMP_SENSOR_ENTRIES)
        return EC_ERR_INVALID_PARAM;

    ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    mtemp = read_mapped_temperature(sensor_idx);

    switch (mtemp) {
        case EC_TEMP_SENSOR_NOT_PRESENT:
        case EC_TEMP_SENSOR_ERROR:
        case EC_TEMP_SENSOR_NOT_POWERED:
        case EC_TEMP_SENSOR_NOT_CALIBRATED:
            return EC_ERR_SENSOR_UNAVAILABLE;
        default:
            mtemp = K_TO_C(mtemp + EC_TEMP_SENSOR_OFFSET);
    }

    libectool_release();

    if (mtemp < 0)
        return EC_ERR_READMEM;
    *temp_out = mtemp;

    return 0;
}

int ec_get_all_temps(int *temps_out, int max_len, int *num_sensors_out) {
    int id, mtemp, ret;
    int count = 0;

    if (!temps_out || max_len < EC_MAX_TEMP_SENSOR_ENTRIES)
        return EC_ERR_INVALID_PARAM;

    ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    for (id = 0; id < EC_MAX_TEMP_SENSOR_ENTRIES; id++) {
        mtemp = read_mapped_temperature(id);
        if (mtemp >= 0) {
            temps_out[id] = K_TO_C(mtemp + EC_TEMP_SENSOR_OFFSET);
            count++;
        } else {
            temps_out[id] = -1;
        }
    }

    libectool_release();

    if (num_sensors_out)
        *num_sensors_out = count;

    return 0;
}

int ec_get_max_temp(int *max_temp) {
    if (!max_temp)
        return EC_ERR_INVALID_PARAM;

    int ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    int t = -1;
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
    return 0;
}

int ec_get_max_non_battery_temp(int *max_temp)
{
    if (!max_temp)
        return EC_ERR_INVALID_PARAM;

    int ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    struct ec_params_temp_sensor_get_info p;
    struct ec_response_temp_sensor_get_info r;
    int t = -1;
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
    return 0;
}

int ec_get_temp_info(int sensor_idx, struct ec_temp_info *info_out) {
    struct ec_response_temp_sensor_get_info temp_r;
    struct ec_params_temp_sensor_get_info temp_p;
    struct ec_params_thermal_get_threshold_v1 thresh_p;
    struct ec_thermal_config thresh_r;
    int mtemp;
    int rc;

    if (!info_out || sensor_idx < 0 || sensor_idx >= EC_MAX_TEMP_SENSOR_ENTRIES)
        return EC_ERR_INVALID_PARAM;

    int ret = libectool_init();
    if (ret < 0)
        return EC_ERR_INIT;

    // Check whether the sensor exists:
    mtemp = read_mapped_temperature(sensor_idx);
    if (mtemp < 0)
        return EC_ERR_SENSOR_UNAVAILABLE;

    // Get sensor info (name, type)
    temp_p.id = sensor_idx;
    rc = ec_command(EC_CMD_TEMP_SENSOR_GET_INFO, 0,
                    &temp_p, sizeof(temp_p),
                    &temp_r, sizeof(temp_r));
    if (rc < 0)
        return EC_ERR_EC_COMMAND;

    strncpy(info_out->sensor_name, temp_r.sensor_name,
            sizeof(info_out->sensor_name) - 1);
    info_out->sensor_name[sizeof(info_out->sensor_name) - 1] = '\0';

    info_out->sensor_type = temp_r.sensor_type;

    info_out->temp = K_TO_C(mtemp + EC_TEMP_SENSOR_OFFSET);

    thresh_p.sensor_num = sensor_idx;
    rc = ec_command(EC_CMD_THERMAL_GET_THRESHOLD, 1,
                    &thresh_p, sizeof(thresh_p),
                    &thresh_r, sizeof(thresh_r));
    if (rc < 0) {
        // Could not read thresholds. Fill with -1 as invalid values.
        info_out->temp_fan_off = -1;
        info_out->temp_fan_max = -1;
    } else {
        info_out->temp_fan_off = K_TO_C(thresh_r.temp_fan_off);
        info_out->temp_fan_max = K_TO_C(thresh_r.temp_fan_max);
    }

    libectool_release();
    return 0;
}
