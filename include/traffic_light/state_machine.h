/**
* @file state_machine.h
 * @brief Public entry point to the traffic light state machine.
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "traffic_light/traffic_light.h"

/**
 * @brief Runs the traffic light cycle until the user stops it.
 *
 * Implements the flowchart:
 *   1. Both lights start in their safe state.
 *   2. Wait for the pedestrian button.
 *   3. Transition through yellow, red, and pedestrian green.
 *   4. Wait for crossing, then flash.
 *   5. Return to the initial state and loop.
 *
 * A button press during a crossing is handled inside the loop:
 *   - Early (vehicle yellow)      -> queued for the next cycle.
 *   - Walk time (pedestrian green) -> join the current window.
 *   - Flash time                  -> queued for the next cycle.
 *
 * @param state Handle created with traffic_light_create().
 * @return true if the user stopped it cleanly, false on error.
 */
bool state_machine_run(TrafficLightState *state);

#endif /* STATE_MACHINE_H */