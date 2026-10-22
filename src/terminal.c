//
// Created by anonymous on 10/22/26.
//
/**
 * @file terminal.c
 * @brief Raw-mode terminal implementation for POSIX and Windows.
 */

#ifdef _WIN32
    #include <windows.h>
#else
    #define _POSIX_C_SOURCE 200809L
    #include <termios.h>
    #include <unistd.h>
#endif

#include "traffic_light/terminal.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    static HANDLE h_stdin = INVALID_HANDLE_VALUE;
    static DWORD original_mode = 0;
#else
    static struct termios original_termios;
#endif

static int raw_mode_active = 0;

static void handle_sigint(int signum)
{
    terminal_restore();
    signal(signum, SIG_DFL);
    raise(signum);
}

void terminal_enter_raw_mode(void)
{
    if (raw_mode_active) {
        return;
    }

#ifdef _WIN32
    /* Get the standard input handle. */
    h_stdin = GetStdHandle(STD_INPUT_HANDLE);
    if (h_stdin == INVALID_HANDLE_VALUE) {
        return;
    }

    /* Save the current console mode. */
    if (!GetConsoleMode(h_stdin, &original_mode)) {
        return;
    }

    /* Disable line buffering (ENABLE_LINE_INPUT) and echo (ENABLE_ECHO_INPUT).
     * Keep ENABLE_PROCESSED_INPUT so Ctrl+C still raises SIGINT. */
    DWORD new_mode = original_mode;
    new_mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    new_mode |= ENABLE_PROCESSED_INPUT;

    if (!SetConsoleMode(h_stdin, new_mode)) {
        return;
    }
#else
    /* POSIX implementation (unchanged) */
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        return;
    }

    struct termios raw = original_termios;
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        return;
    }
#endif

    raw_mode_active = 1;
    atexit(terminal_restore);
    signal(SIGINT, handle_sigint);
}

void terminal_restore(void)
{
    if (!raw_mode_active) {
        return;
    }

#ifdef _WIN32
    SetConsoleMode(h_stdin, original_mode);
#else
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
#endif

    raw_mode_active = 0;
    putchar('\n');
    fflush(stdout);
}