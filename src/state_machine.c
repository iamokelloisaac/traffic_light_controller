/**
 * @file state_machine.c
 * @brief Traffic light cycle with mid-crossing input handling.
 *
 * Safety timing is never altered by input. Input only affects
 * *when the next cycle starts* and *what the pedestrian is told*.
 */

#include "traffic_light/state_machine.h"

#include <stdio.h>   /* For printf, snprintf, fprintf. */

#include "traffic_light/input.h"
#include "traffic_light/timing.h"
#include "traffic_light/ui.h"

/* How often we check for input during a sleep. One second is a
 * good balance between responsiveness and CPU usage. */
#define POLL_INTERVAL_SECONDS 1

/* Put the lights into the safe starting state. */
static void enter_initial_state(TrafficLightState *state)
{
    traffic_light_set_vehicle(state, LIGHT_GREEN);
    traffic_light_set_pedestrian(state, LIGHT_RED);
    traffic_light_print_state(state);
}

/* Sleep for up to `seconds`, returning early if the user presses
 * anything. Returns the event that woke us up, or INPUT_NONE if
 * the sleep completed normally. */
static InputEvent sleep_with_polling(int seconds)
{
    for (int elapsed = 0; elapsed < seconds; ++elapsed) {
        InputEvent event = input_poll();
        if (event != INPUT_NONE) {
            return event;
        }
        timing_sleep_seconds(POLL_INTERVAL_SECONDS);
    }
    return INPUT_NONE;
}

/* Wait indefinitely for a pedestrian press or a stop request. */
static bool wait_for_pedestrian_or_stop()
{
    /* Draw a small framed prompt. */
    ui_divider();
    ui_box_line(UI_RESET, "Waiting for pedestrian button...");
    ui_box_line(UI_DIM,   "any key = press    q = stop");
    ui_divider();

    for (;;) {
        InputEvent event = input_poll();

        if (event == INPUT_STOP) {
            ui_log(UI_RED, "Stop requested by user.");
            return false;
        }

        if (event == INPUT_BUTTON) {
            ui_log(UI_GREEN, "Pedestrian button detected.");
            return true;
        }

        /* No input. Sleep briefly so we do not burn a CPU core
         * spinning this loop at millions of iterations per second. */
        timing_sleep_seconds(POLL_INTERVAL_SECONDS);
    }
}

/* Run one full crossing sequence. If a button is pressed during
 * the sequence, *queue_next is set so the caller can skip the
 * wait step and start the next cycle immediately. */
static InputEvent run_crossing_sequence(TrafficLightState *state,
                                        bool *queue_next)
{
    *queue_next = false;

    printf("\n");
    ui_log(UI_CYAN, "Starting crossing sequence");

    /* Step 1: vehicle YELLOW for three seconds. */
    traffic_light_set_vehicle(state, LIGHT_YELLOW);
    traffic_light_print_state(state);

    InputEvent event = sleep_with_polling(YELLOW_LIGHT_DURATION_SECONDS);
    if (event == INPUT_STOP) {
        return INPUT_STOP;
    }
    if (event == INPUT_BUTTON) {
        /* Someone arrived too early to walk. Queue them for the
         * next full cycle; do not shorten the yellow phase. */
        ui_log(UI_YELLOW, "New pedestrian arrived early — please wait.");
        *queue_next = true;
    }

    /* Step 2: vehicle RED and pedestrian GREEN. */
    traffic_light_set_vehicle(state, LIGHT_RED);
    traffic_light_set_pedestrian(state, LIGHT_GREEN);
    traffic_light_print_state(state);

    /* Step 3: pedestrians walk for ten seconds, polled each second. */
    printf("     ");
    ui_log(UI_GREEN, "Pedestrians may cross (10s)");

    for (int elapsed = 0; elapsed < PEDESTRIAN_CROSS_DURATION_SECONDS; ++elapsed) {
        event = input_poll();

        if (event == INPUT_STOP) {
            return INPUT_STOP;
        }
        if (event == INPUT_BUTTON) {
            /* It is still safe to walk, so a late arrival can
             * join the current window. Tell them how long is left. */
            char message[80];
            int remaining = PEDESTRIAN_CROSS_DURATION_SECONDS - elapsed;
            snprintf(message, sizeof message,
                     "Additional pedestrian — join now (%d s left).",
                     remaining);
            ui_log(UI_GREEN, message);
        }

        timing_sleep_seconds(POLL_INTERVAL_SECONDS);
    }

    /* Step 4: pedestrian FLASHING for five seconds. */
    traffic_light_set_pedestrian(state, LIGHT_FLASHING);
    traffic_light_print_state(state);
    printf("     ");
    ui_log(UI_YELLOW, "Pedestrian light flashing (5s)");

    for (int elapsed = 0; elapsed < PEDESTRIAN_FLASH_DURATION_SECONDS; ++elapsed) {
        event = input_poll();

        if (event == INPUT_STOP) {
            return INPUT_STOP;
        }
        if (event == INPUT_BUTTON) {
            /* The window is closing. Extending it would be unsafe,
             * so queue the press for the next full cycle. */
            ui_log(UI_YELLOW, "Too late to cross — queued for the next cycle.");
            *queue_next = true;
        }

        timing_sleep_seconds(POLL_INTERVAL_SECONDS);
    }

    /* Step 5: pedestrian RED and vehicle GREEN — back to start. */
    traffic_light_set_pedestrian(state, LIGHT_RED);
    traffic_light_set_vehicle(state, LIGHT_GREEN);
    traffic_light_print_state(state);

    ui_log(UI_CYAN, "Crossing sequence complete");
    printf("\n");

    return INPUT_NONE;
}

bool state_machine_run(TrafficLightState *state)
{
    /* Guard: refuse to run without a valid state handle. */
    if (state == NULL) {
        fprintf(stderr, "Error: state_machine_run received NULL state.\n");
        return false;
    }

    /* Paint the top of the box once at startup. */
    ui_clear_screen();
    ui_box_border('T');
    ui_box_title("TRAFFIC LIGHT CONTROLLER");
    ui_box_border('M');

    enter_initial_state(state);

    ui_box_border('B');
    printf("\n");

    /* If a late pedestrian is queued, we skip the wait step and
     * start the next cycle immediately. */
    bool queue_next = false;

    for (;;) {
        if (!queue_next) {
            bool button_pressed = wait_for_pedestrian_or_stop();
            if (!button_pressed) {
                ui_log(UI_RED, "System stopped manually.");
                return true;
            }
        } else {
            ui_log(UI_GREEN, "Queued pedestrian — starting next cycle.");
            queue_next = false;
        }

        bool next_queued = false;
        InputEvent result = run_crossing_sequence(state, &next_queued);

        if (result == INPUT_STOP) {
            ui_log(UI_RED, "System stopped manually.");
            return true;
        }

        queue_next = next_queued;
    }
}
