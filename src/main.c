/**
 * @file main.c
 * @brief Program entry point.
 *
 * Responsibilities, in order:
 *   1. Configure the terminal for the current platform.
 *   2. Switch stdin into raw mode so keystrokes arrive immediately.
 *   3. Create the traffic light state (this file owns it).
 *   4. Run the state machine until it returns.
 *   5. Destroy the state, restore the terminal, and print the result.
 *
 * Keeping the list this short makes it easy to check at a glance
 * that every resource has a matching cleanup.
 */

#include <stdio.h>   /* For printf and fprintf. */
#include <stdlib.h>  /* For EXIT_SUCCESS and EXIT_FAILURE. */

#include "traffic_light/platform.h"
#include "traffic_light/state_machine.h"
#include "traffic_light/terminal.h"
#include "traffic_light/traffic_light.h"
#include "traffic_light/ui.h"

int main()
{
    /* Enable ANSI + UTF-8 on Windows. No-op on Kali. */
    platform_init();

    /* Switch stdin to raw mode. This also registers handlers that
     * restore the terminal on normal exit and on Ctrl+C. */
    terminal_enter_raw_mode();

    /* Allocate the traffic light state. Ownership stays in main():
     * nobody else frees this pointer. */
    TrafficLightState *state = traffic_light_create();
    if (state == NULL) {
        terminal_restore();
        fprintf(stderr, "Failed to allocate traffic light state.\n");
        return EXIT_FAILURE;
    }

    /* Short welcome banner. */
    printf(UI_BOLD UI_CYAN "Traffic Light Controller" UI_RESET "\n");
    printf(UI_DIM "Controls: any key = button, q = stop" UI_RESET "\n\n");

    /* Run the state machine until it returns. It returns true on a
     * clean stop, false on an internal error. */
    bool stopped_manually = state_machine_run(state);

    /* Clean up before printing the farewell message, so the shell
     * prompt appears on a fresh line. */
    traffic_light_destroy(state);
    terminal_restore();

    if (stopped_manually) {
        printf("\n" UI_BOLD UI_GREEN "Shutdown complete. Goodbye!" UI_RESET "\n");
        return EXIT_SUCCESS;
    }

    fprintf(stderr, "System terminated due to an error.\n");
    return EXIT_FAILURE;
}
