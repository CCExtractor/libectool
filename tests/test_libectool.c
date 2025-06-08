#include <stdio.h>
#include <stdbool.h>
#include "libectool.h"

void print_menu() {
    printf("\n=== libectool Testing CLI ===\n");
    printf("1. Check if on AC power\n");
    printf("2. Pause fan control\n");
    printf("3. Set fan speed\n");
    printf("4. Get max temperature\n");
    printf("5. Get max non-battery temperature\n");
    printf("0. Exit\n");
    printf("Choose an option: ");
}

int main() {
    int choice;
    int speed;

    while (1) {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            // clear invalid input
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input. Try again.\n");
            continue;
        }

        switch (choice) {
            case 1: {
                bool ac = is_on_ac();
                printf("is_on_ac() = %d\n", ac);
                break;
            }
            case 2:
                printf("Pausing fan control...\n");
                pause_fan_control();
                break;
            case 3:
                printf("Enter fan speed (0-100): ");
                if (scanf("%d", &speed) == 1) {
                    set_fan_speed(speed);
                } else {
                    printf("Invalid speed.\n");
                    // clear invalid input
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF);
                }
                break;
            case 4: {
                float max_temp = get_max_temperature();
                printf("Max temperature = %.2f C\n", max_temp);
                break;
            }
            case 5: {
                float max_non_batt_temp = get_max_non_battery_temperature();
                printf("Max non-battery temperature = %.2f C\n", max_non_batt_temp);
                break;
            }
            case 0:
                printf("Exiting.\n");
                return 0;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }
}
