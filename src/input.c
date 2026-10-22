//
// Created by anonymous on 10/22/26.
//
/**
 * @file input.c
 * @brief Non-blocking keystroke polling for POSIX and Windows.
 */

#ifdef _WIN32
    #include <conio.h>
#else
    #define _POSIX_C_SOURCE 200809L
    #include <sys/select.h>
    #include <unistd.h>
#endif

#include "traffic_light/input.h"

#ifndef _WIN32
/* POSIX helpers are only compiled on non-Windows platforms. */
static int stdin_ready(void)
{
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    struct timeval timeout = {0, 0};
    return select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);
}

static int read_one_byte(void)
{
    unsigned char byte = 0;
    ssize_t count = read(STDIN_FILENO, &byte, 1);
    if (count <= 0) {
        return -1;
    }
    return (int)byte;
}
#endif

void input_clear_buffer(void)
{
#ifdef _WIN32
    while (_kbhit()) {
        (void)_getch();
    }
#else
    while (stdin_ready() > 0) {
        if (read_one_byte() < 0) {
            break;
        }
    }
#endif
}

InputEvent input_poll(void)
{
    int byte = -1;

#ifdef _WIN32
    if (!_kbhit()) {
        return INPUT_NONE;
    }
    byte = _getch();
#else
    if (stdin_ready() <= 0) {
        return INPUT_NONE;
    }
    byte = read_one_byte();
    if (byte < 0) {
        return INPUT_NONE;
    }
#endif

    InputEvent event = (byte == 'q' || byte == 'Q')
                     ? INPUT_STOP
                     : INPUT_BUTTON;

    input_clear_buffer();
    return event;
}