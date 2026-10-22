//
// Created by anonymous on 10/22/26.
//
/**
 * @file input.c
 * @brief Non-blocking keystroke polling using select().
 *
 * Junior developer note: select() is a POSIX function that waits
 * for one of a set of file descriptors to become readable. Using
 * it with a zero timeout lets us "peek" at stdin without blocking.
 *
 * Reading from stdin directly with read() (instead of getchar())
 * avoids stdio's own buffering, which could hide keystrokes from
 * select().
 */

#define POSIX_C_SOURCE 200809L

#include "traffic_light/input.h"

#include <sys/select.h>  /* For fd_set, FD_ZERO, FD_SET, select. */
#include <unistd.h>      /* For read and STDIN_FILENO. */

/**
 * @brief Checks whether stdin has data ready to read.
 *
 * @return 1 if data is ready, 0 if not, -1 on error.
 */
static int stdin_ready()
{
    /* fd_set is a bitmask of file descriptors. We only watch stdin. */
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    /* A zero timeout tells select to return immediately. */
    struct timeval timeout = {0, 0};

    /* The first argument is "highest descriptor + 1" — a POSIX
     * convention. Passing NULL for write and exception sets means
     * we only care about readability. */
    return select(STDIN_FILENO + 1, &read_fds, nullptr, nullptr, &timeout);
}

/**
 * @brief Reads a single byte from stdin, or -1 if nothing is ready.
 */
static int read_one_byte()
{
    unsigned char byte = 0;
    ssize_t count = read(STDIN_FILENO, &byte, 1);
    if (count <= 0) {
        return -1;
    }
    return (int)byte;
}

void input_clear_buffer()
{
    /* Drain everything currently readable without blocking. */
    while (stdin_ready() > 0) {
        if (read_one_byte() < 0) {
            break;
        }
    }
}

InputEvent input_poll()
{
    if (stdin_ready() <= 0) {
        return INPUT_NONE;
    }

    int byte = read_one_byte();
    if (byte < 0) {
        return INPUT_NONE;
    }

    /* Classify: 'q' or 'Q' is STOP, anything else is BUTTON. */
    InputEvent event = (byte == 'q' || byte == 'Q')
                     ? INPUT_STOP
                     : INPUT_BUTTON;

    /* Drain anything else typed at the same time so a single burst
     * of keystrokes doesn't become many events. */
    input_clear_buffer();
    return event;
}