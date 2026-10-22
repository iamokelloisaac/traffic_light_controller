/**
 * @file traffic_light.c
 * @brief Implementation of the opaque traffic light state.
 *
 * This is the ONLY file that knows what fields TrafficLightState
 * actually contains. Everything else treats it as a black box.
 */

#include "traffic_light/traffic_light.h"

#include <stdio.h>
#include <stdlib.h>

#include "traffic_light/ui.h"

/**
 * The full definition of the opaque type.
 *
 * In the header, TrafficLightState was declared but not defined.
 * That's what "opaque" means: the compiler knows the name but not
 * the size or the fields. Here, inside the implementation file,
 * we fill in the details.
 *
 * Because the struct definition is not in the header, no other
 * file can accidentally read or write these fields directly.
 */
struct [[maybe_unused]] TrafficLightState {
    LightState vehicle_light;
    LightState pedestrian_light;
};

TrafficLightState *traffic_light_create()
{
    /* calloc allocates memory and fills it with zeros.
     * Because LIGHT_RED == 0 in the enum, a zeroed struct means
     * both lights start RED without needing an explicit initializer.
     * This is a small but deliberate design choice. */
    return calloc(1, sizeof(TrafficLightState));
}

void traffic_light_destroy(TrafficLightState *state)
{
    /* free(NULL) is safe: it does nothing. So we don't need an
     * `if (state != NULL)` check here. */
    free(state);
}

LightState traffic_light_vehicle(const TrafficLightState *state)
{
    return state->vehicle_light;
}

LightState traffic_light_pedestrian(const TrafficLightState *state)
{
    return state->pedestrian_light;
}

/* Hidden helper: maps a LightState to a matching ANSI colour. */
static const char *light_state_to_color(LightState state)
{
    switch (state) {
    case LIGHT_RED:      return UI_RED;
    case LIGHT_YELLOW:   return UI_YELLOW;
    case LIGHT_GREEN:    return UI_GREEN;
    case LIGHT_FLASHING: return UI_CYAN;   /* Cyan stands out from green. */
    default:             return UI_RESET;
    }
}

const char *light_state_to_string(LightState state)
{
    switch (state) {
    case LIGHT_RED:      return "RED";
    case LIGHT_YELLOW:   return "YELLOW";
    case LIGHT_GREEN:    return "GREEN";
    case LIGHT_FLASHING: return "FLASHING";
    default:             return "UNKNOWN";
    }
}

void traffic_light_print_state(const TrafficLightState *state)
{
    /* Guard clause: dereferencing NULL would crash the program. */
    if (state == NULL) {
        return;
    }

    /* ui_both_lights is declared in ui.h. It handles all the
     * colours and formatting, so this function stays short. */
    ui_both_lights(light_state_to_color(state->vehicle_light),
                   light_state_to_string(state->vehicle_light),
                   light_state_to_color(state->pedestrian_light),
                   light_state_to_string(state->pedestrian_light));

    putchar('\n');
}

void traffic_light_set_vehicle(TrafficLightState *state, LightState new_state)
{
    if (state == NULL) {
        return;
    }

    state->vehicle_light = new_state;

    /* Build the log message in a local buffer, then hand it to ui_log.
     * snprintf is used instead of sprintf because it cannot overflow
     * the buffer: it stops at sizeof(buffer) - 1 characters. */
    char buffer[64];
    snprintf(buffer, sizeof buffer, "Vehicle light -> %s",
             light_state_to_string(new_state));

    ui_log(light_state_to_color(new_state), buffer);
}

void traffic_light_set_pedestrian(TrafficLightState *state, LightState new_state)
{
    if (state == NULL) {
        return;
    }

    state->pedestrian_light = new_state;

    char buffer[64];
    snprintf(buffer, sizeof buffer, "Pedestrian light -> %s",
             light_state_to_string(new_state));

    ui_log(light_state_to_color(new_state), buffer);
}