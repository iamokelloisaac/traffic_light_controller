/**
* @file terminal.h
 * @brief Controls the terminal's input mode.
 *
 * By default, the terminal operates in "canonical mode": it waits
 * for the user to press Enter before delivering anything to our
 * program. That makes it impossible to react to a single keypress.
 *
 * This module switches the terminal to "raw mode", where every
 * keystroke is delivered immediately.
 *
 * Restoring the terminal is essential: if we leave it in raw mode,
 * the shell will behave strangely after the program exits.
 */

#ifndef TERMINAL_H
#define TERMINAL_H

/**
 * @brief Switches stdin to raw mode.
 *
 * Also registers handlers that restore the terminal on normal exit
 * and on Ctrl+C, so the user never has to run `stty sane` to fix
 * their shell.
 *
 * Safe to call multiple times; only the first call does anything.
 */
void terminal_enter_raw_mode();

/**
 * @brief Restores the terminal to its original settings.
 *
 * Called automatically at exit and on SIGINT. You may also call it
 * manually if you want to print something after the program ends
 * but before the shell prompt returns.
 */
void terminal_restore();

#endif /* TERMINAL_H */