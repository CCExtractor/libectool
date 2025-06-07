#include <stdio.h>
#include "libectool.h"

int main() {
    printf("Testing libectool...\n");

    // Test is_on_ac
    bool ac = is_on_ac();
    printf("is_on_ac() = %d\n", ac);

    // // Test fan control functions
    // printf("Pausing fan control...\n");
    // pause_fan_control();

    // printf("Setting fan speed to 50%%...\n");
    // set_fan_speed(50);

    // Test temperature functions
    float max_temp = get_max_temperature();
    printf("Max temperature = %.2f C\n", max_temp);

    float max_non_batt_temp = get_max_non_battery_temperature();
    printf("Max non-battery temperature = %.2f C\n", max_non_batt_temp);

    printf("Test complete.\n");
    return 0;
}
