/**
* @file platform.c
 * @brief Terminal setup per platform.
 *
 * On Windows 10+, ANSI escape sequences exist but must be enabled
 * explicitly. UTF-8 output also needs to be enabled by setting the
 * console code page.
 *
 * On Kali Linux, both are enabled by default, so the function does
 * nothing.
 */

#include "traffic_light/platform.h"

/* The _WIN32 macro is defined by the compiler when targeting Windows.
 * It is the standard way to write platform-specific code in C. */
#ifdef _WIN32
    #include <windows.h>
#endif

void platform_init()
{
#ifdef _WIN32
    /* Get the standard output handle (the current console). */
    HANDLE h_out = GetStdHandle(STD_OUTPUT_HANDLE);

    if (h_out != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;

        if (GetConsoleMode(h_out, &mode)) {
            /* Enable the Virtual Terminal flag so ANSI escapes work. */
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(h_out, mode);
        }
    }

    /* Set the console code page to UTF-8 so our box-drawing
     * characters are displayed correctly. */
    SetConsoleOutputCP(CP_UTF8);
#else
    /* On Kali Linux, ANSI and UTF-8 are already enabled. Nothing
     * to do. The cast silences an "empty function" warning. */
    (void)0;
#endif
}