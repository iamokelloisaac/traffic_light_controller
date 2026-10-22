/**
 * @file traffic_light.h
 * @brief Public API for the traffic light state.
 *
 * This header is the "menu" of what the traffic light module can do.
 * It does NOT reveal how the state is stored internally. That means
 * the implementation can change without breaking code that uses it.
 *
 * This technique is called an "opaque type" (or "opaque pointer").
 * See the typedef below for details.
 */

#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include <stdbool.h>

/**
 * @brief Opaque handle to the traffic light state.
 *
 * A junior developer might wonder: where is the struct?
 *
 * The answer: `typedef struct TrafficLightState TrafficLightState;`
 * declares that a struct called TrafficLightState *exists*, but it
 * does not define its members. Only traffic_light.c defines the
 * members. Everyone else can hold a pointer to it, but they cannot
 * peek inside.
 *
 * Why do this? Because it lets us:
 *   1. Change the struct's layout without recompiling other files.
 *   2. Enforce that all access goes through our functions, where
 *      we can add safety checks and logging.
 *   3. Make the ownership rules crystal clear: whoever calls
 *      traffic_light_create() must also call traffic_light_destroy().
 */
typedef struct TrafficLightState TrafficLightState;

/**
 * @brief One light's colour.
 *
 * Enums are safer than plain ints because the compiler can warn
 * if you write something invalid like `LIGHT_PURPLE`.
 */
typedef enum {
    LIGHT_RED = 0,      /* 0 so that a zeroed struct means "all red". */
    LIGHT_YELLOW,       /* Automatically 1. */
    LIGHT_GREEN,        /* Automatically 2. */
    LIGHT_FLASHING      /* Automatically 3; pedestrian-only. */
} LightState;

/**
 * @brief Creates a new traffic light state with both lights RED.
 *
 * @return A pointer to the new state, or NULL if memory allocation
 *         failed. The caller owns the memory and must free it with
 *         traffic_light_destroy().
 */
TrafficLightState *traffic_light_create(void);

/**
 * @brief Frees a traffic light state.
 *
 * Safe to call with NULL (does nothing), which matches the
 * behaviour of free().
 *
 * @param state The state to free.
 */
void traffic_light_destroy(TrafficLightState *state);

/**
 * @brief Reads the current vehicle light colour.
 *
 * @param state The state to read. Must not be NULL.
 * @return The vehicle light's current colour.
 */
LightState traffic_light_vehicle(const TrafficLightState *state);

/**
 * @brief Reads the current pedestrian light colour.
 *
 * @param state The state to read. Must not be NULL.
 * @return The pedestrian light's current colour.
 */
LightState traffic_light_pedestrian(const TrafficLightState *state);

/**
 * @brief Sets the vehicle light to a new colour and logs the change.
 *
 * @param state     The state to modify. Must not be NULL.
 * @param new_state The new colour.
 */
void traffic_light_set_vehicle(TrafficLightState *state, LightState new_state);

/**
 * @brief Sets the pedestrian light to a new colour and logs the change.
 *
 * @param state     The state to modify. Must not be NULL.
 * @param new_state The new colour.
 */
void traffic_light_set_pedestrian(TrafficLightState *state, LightState new_state);

/**
 * @brief Prints both lights side by side on one line.
 *
 * @param state The state to print. Must not be NULL.
 */
void traffic_light_print_state(const TrafficLightState *state);

/**
 * @brief Converts a LightState value to a human-readable string.
 *
 * @param state The light state to convert.
 * @return A string literal (e.g. "RED"), or "UNKNOWN" if the
 *         value is not one of the four valid enum constants.
 */
const char *light_state_to_string(LightState state);

#endif /* TRAFFIC_LIGHT_H */