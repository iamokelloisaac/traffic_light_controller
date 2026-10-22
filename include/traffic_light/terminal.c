/**
 * @file terminal.c
 * @brief Raw-mode terminal implementation for POSIX systems.
 *
 * Junior developer note: this file uses <termios.h>, which is the
 * POSIX way to talk to the terminal driver. It is not available on
 * native Windows. The project targets Kali Linux (and WSL), so
 * POSIX is fine here. If you ever port to native Windows, you
 * would replace the termios calls with Win32 Console API calls.
 */

#define POSIX_C_SOURCE 200809L

#include "traffic_light/terminal.h"

#include <signal.h>   /* For signal(), SIGINT, SIG_DFL, raise(). */
#include <stdio.h>    /* For putchar, fflush. */
#include <stdlib.h>   /* For atexit(). */
#include <termios.h>  /* For struct termios, tcgetattr, tcsetattr. */
#include <unistd.h>   /* For STDIN_FILENO. */

/* A copy of the terminal settings as they were before we touched
 * anything. Used by terminal_restore to put things back exactly. */
static struct termios original_termios;

/* Tracks whether we are currently in raw mode, so restore is
 * idempotent and can be called any number of times. */
static int raw_mode_active = 0;

/**
 * @brief Signal handler for Ctrl+C.
 *
 * We restore the terminal, then re-raise the signal with the
 * default handler. That ensures the process terminates with the
 * normal "interrupted" status, so shell scripts that ran our
 * program still see the interrupt.
 */
static void handle_sigint(int signum)
{
    terminal_restore();
    signal(signum, SIG_DFL);
    raise(signum);
}

void terminal_enter_raw_mode()
{
    /* If we already entered raw mode, do nothing. */
    if (raw_mode_active) {
        return;
    }

    /* tcgetattr reads the current settings. If stdin is not a
     * terminal (for example, we are piped from a file), this fails.
     * In that case we simply skip raw mode and let the caller use
     * normal buffered input. */
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        return;
    }

    /* Modify a copy, not the original: we want to change only a
     * few flags and keep everything else the way the user's shell
     * configured it.
     *
     *   ICANON: line buffering. Turning it off delivers keys one by one.
     *   ECHO:   local echo. Turning it off stops the terminal from
     *           printing every key we type.
     *   ISIG:   keeps Ctrl+C working (we leave it on).
     */
    struct termios raw = original_termios;
    raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);

    /* VMIN and VTIME control how read() behaves.
     * Setting both to 0 means "return immediately, even if no bytes
     * are available", which matches our non-blocking polling. */
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;

    /* Apply the new settings. TCSAFLUSH discards any pending input
     * that the user typed before we switched modes. */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        return;
    }

    raw_mode_active = 1;

    /* Ensure restoration happens automatically on normal exit
     * and on Ctrl+C. */
    atexit(terminal_restore);
    signal(SIGINT, handle_sigint);
}

void terminal_restore()
{
    /* Nothing to restore if we never changed anything. */
    if (!raw_mode_active) {
        return;
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
    raw_mode_active = 0;

    /* Print a newline so the shell prompt appears on its own line
     * instead of being appended to our last output. */
    putchar('\n');
    fflush(stdout);
}
