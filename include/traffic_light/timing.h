/**
* @file timing.h
 * @brief Phase durations and a safe sleep helper.
 *
 * This module is deliberately small: it has no dependencies on any
 * other project module, so any file that needs a sleep can include
 * it without pulling in unrelated code.
 */

#ifndef TIMING_H
#define TIMING_H

/* How many seconds each phase of the traffic cycle lasts.
 * Defining them here means the whole system can be tuned from one
 * place, and the state machine stays readable. */

#define YELLOW_LIGHT_DURATION_SECONDS      3   /* Vehicle yellow. */
#define PEDESTRIAN_CROSS_DURATION_SECONDS 10   /* Pedestrian green. */
#define PEDESTRIAN_FLASH_DURATION_SECONDS  5   /* Pedestrian flashing. */

/**
 * @brief Sleeps for `seconds` seconds.
 *
 * The implementation handles EINTR automatically: if a signal
 * wakes us early (e.g. Ctrl+C), we resume the sleep for the
 * remaining time instead of returning prematurely.
 *
 * This guarantees callers can rely on the full duration elapsing.
 *
 * @param seconds Number of seconds to sleep. Non-positive values
 *                return immediately.
 */
void timing_sleep_seconds(int seconds);

#endif /* TIMING_H */
