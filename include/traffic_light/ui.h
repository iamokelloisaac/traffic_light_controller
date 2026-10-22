/**
 * @file ui.h
 * @brief Terminal formatting helpers: colours, boxes, and messages.
 *
 * This module is intentionally isolated from the traffic light
 * logic. It knows how to draw things, but knows nothing about
 * lights or timing. Swapping the UI for a GUI later would only
 * require rewriting this module.
 *
 * Target: modern ANSI/UTF-8 terminals (Kali default, Windows 10+
 * after platform_init()).
 */

#ifndef UI_H
#define UI_H

/* ANSI escape codes. Each one is a string that the terminal
 * interprets as an instruction (change colour, go bold, etc.)
 * rather than printing as text.
 *
 * "\033" is the escape character (octal 33 = ASCII 27). */

#define UI_RESET     "\033[0m"    /* Turn off all colours and styles. */
#define UI_BOLD      "\033[1m"    /* Bold / bright. */
#define UI_DIM       "\033[2m"    /* Faint. */
#define UI_RED       "\033[31m"   /* Foreground red. */
#define UI_GREEN     "\033[32m"   /* Foreground green. */
#define UI_YELLOW    "\033[33m"   /* Foreground yellow. */
#define UI_CYAN      "\033[36m"   /* Foreground cyan. */

/* The visible interior width of our rounded boxes. */
#define UI_BOX_WIDTH 56

/* Clear the screen and move the cursor to the top-left. */
void ui_clear_screen();

/* A dim horizontal dashed line, as wide as a box. */
void ui_divider();

/* One border row of a box.
 *   'T' = top border
 *   'M' = middle border
 *   'B' = bottom border
 */
void ui_box_border(char kind);

/* A line of text inside a box, coloured and padded to fit. */
void ui_box_line(const char *color, const char *text);

/* A bold, cyan, centred title inside a box. */
void ui_box_title(const char *title);

/* A small bullet followed by a coloured message. Used for logs. */
void ui_log(const char *color, const char *text);

/* Two lamp indicators on one line, for vehicle + pedestrian. */
void ui_both_lights(const char *vehicle_color,    const char *vehicle_label,
                    const char *pedestrian_color, const char *pedestrian_label);

#endif /* UI_H */