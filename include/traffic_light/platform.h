/**
* @file platform.h
 * @brief One-time terminal setup for supported platforms.
 *
 * Windows 10+ needs an explicit opt-in to interpret ANSI escape
 * sequences and to accept UTF-8. Kali Linux does not.
 *
 * Having this in a dedicated module means main() does not need to
 * know about #ifdef _WIN32 or any Win32 API. It just calls
 * platform_init() once and moves on.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

/**
 * @brief Configures the terminal for the current platform.
 *
 * Call exactly once at the very start of main(), before any
 * output or raw-mode toggling.
 */
void platform_init();

#endif /* PLATFORM_H */