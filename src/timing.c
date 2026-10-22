/**
 * @file timing.c
 * @brief Implementation of the sleep helper.
 *
 * Junior developer note: nanosleep() is a POSIX function, not a
 * standard C function. To use it, we have to request POSIX support
 * from the C library by defining a "feature test macro" BEFORE we
 * include any standard headers.
 *
 * The macro name is _POSIX_C_SOURCE, and 200809L means "I want the
 * POSIX.1-2008 feature set". Every POSIX function we use in this
 * project is available at that level.
 *
 * If you ever see an "implicit declaration of function" error for
 * something like nanosleep, select, or tcgetattr, the first thing
 * to check is whether this macro is defined at the top of the file.
 */

#define _POSIX_C_SOURCE 200809L

#include "traffic_light/timing.h"

#include <errno.h>   /* For errno and the EINTR constant. */
#include <stdio.h>   /* For perror, used to report unexpected errors. */
#include <time.h>    /* For struct timespec and nanosleep. */

void timing_sleep_seconds(int seconds)
{
    /* A non-positive duration means "no sleep needed". */
    if (seconds <= 0) {
        return;
    }

    /* nanosleep takes a struct timespec, not a plain int.
     * We convert the requested whole seconds into one. */
    struct timespec request = {
        .tv_sec  = seconds,
        .tv_nsec = 0
    };

    /* If the sleep is interrupted by a signal, nanosleep fills this
     * struct with the time that was still remaining. We use it to
     * resume the sleep for the correct duration. */
    struct timespec remaining = {0};

    /* nanosleep returns -1 on error. The two cases we must handle:
     *   - EINTR: a signal interrupted us. Restart with the remaining time.
     *   - anything else: report and give up. */
    while (nanosleep(&request, &remaining) == -1) {
        if (errno != EINTR) {
            perror("nanosleep");
            return;
        }
        /* Copy the remaining time back into request so the next
         * nanosleep call sleeps only for what is left. */
        request = remaining;
    }
}
