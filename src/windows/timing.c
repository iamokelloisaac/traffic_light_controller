//
// Created by anonymous on 10/22/26.
//
/**
 * @file timing.c
 * @brief Implementation of the sleep helper.
 */

#ifdef _WIN32
    #include <windows.h>
#else
    #define _POSIX_C_SOURCE 200809L
    #include <errno.h>
    #include <time.h>
#endif

#include "traffic_light/timing.h"
#include <stdio.h>

void timing_sleep_seconds(int seconds)
{
    if (seconds <= 0) {
        return;
    }

#ifdef _WIN32
    /* Windows Sleep() takes milliseconds. 1 second = 1000 ms. */
    Sleep((DWORD)(seconds * 1000));
#else
    struct timespec request = {
        .tv_sec  = seconds,
        .tv_nsec = 0
    };
    struct timespec remaining = {0};

    while (nanosleep(&request, &remaining) == -1) {
        if (errno != EINTR) {
            perror("nanosleep");
            return;
        }
        request = remaining;
    }
#endif
}