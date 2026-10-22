//
// Created by anonymous on 10/22/26.
//
/**
 * @file ui.c
 * @brief Terminal formatting implementation.
 *
 * All functions here assume an ANSI/UTF-8 capable terminal. That is
 * the default on Kali, and platform_init() enables it on Windows 10+.
 */

#include "traffic_light/ui.h"

#include <stdio.h>   /* For stdout, putchar, printf, fputs, fflush. */
#include <string.h>  /* For strlen. */

/* Sequences used only inside this file. Not exposed in the header. */
#define UI_CLEAR "\033[2J"   /* Erase the entire screen. */
#define UI_HOME  "\033[H"    /* Move cursor to the top-left corner. */

void ui_clear_screen()
{
    /* Send erase + home in one write, then flush so the screen
     * updates immediately (stdout is line-buffered when attached
     * to a terminal). */
    fputs(UI_CLEAR UI_HOME, stdout);
    fflush(stdout);
}

/* Helper: prints `count` copies of a single character. */
static void print_repeat(char c, int count)
{
    for (int i = 0; i < count; ++i) {
        putchar(c);
    }
}

void ui_divider()
{
    /* Wrap the dashes in DIM...RESET so they appear grey. */
    printf(UI_DIM);
    print_repeat('-', UI_BOX_WIDTH + 2);
    printf(UI_RESET "\n");
}

void ui_box_border(char kind)
{
    /* Border characters are UTF-8 sequences. The C compiler turns
     * \u256d into the right bytes for the terminal to decode. */
    const char *left;
    const char *right;

    switch (kind) {
    case 'T':
        left  = "\u256d";   /* ╭ */
        right = "\u256e";   /* ╮ */
        break;
    case 'M':
        left  = "\u251c";   /* ├ */
        right = "\u2524";   /* ┤ */
        break;
    default:                /* 'B' */
        left  = "\u2570";   /* ╰ */
        right = "\u256f";   /* ╯ */
        break;
    }

    /* Draw the whole row: left corner, horizontal run, right corner. */
    printf(UI_DIM "%s" UI_RESET, left);

    printf(UI_DIM);
    for (int i = 0; i < UI_BOX_WIDTH; ++i) {
        printf("\u2500");   /* ─ */
    }
    printf(UI_RESET);

    printf(UI_DIM "%s" UI_RESET "\n", right);
}

/* Helper: write `text` and pad with spaces up to UI_BOX_WIDTH.
 * Truncates if the text is too long. */
static void print_padded(const char *text)
{
    size_t length = strlen(text);
    if (length > UI_BOX_WIDTH) {
        length = UI_BOX_WIDTH;
    }

    fwrite(text, 1, length, stdout);

    for (size_t i = length; i < UI_BOX_WIDTH; ++i) {
        putchar(' ');
    }
}

void ui_box_line(const char *color, const char *text)
{
    /* A box line looks like:
     *   │ <coloured padded text> │
     * with dim vertical bars on each side. */

    printf(UI_DIM "\u2502" UI_RESET);   /* Left border. */
    putchar(' ');                        /* Inner padding. */
    printf("%s", color);                 /* Set text colour. */
    print_padded(text);                  /* Text + filler. */
    printf(UI_RESET);                    /* Reset colour. */
    putchar(' ');                        /* Inner padding. */
    printf(UI_DIM "\u2502" UI_RESET "\n"); /* Right border. */
}

void ui_box_title(const char *title)
{
    /* Centre the title inside the box by splitting leftover space
     * evenly on each side. If the split isn't exact, the right
     * side gets the extra space. */
    size_t length = strlen(title);
    if (length > UI_BOX_WIDTH) {
        length = UI_BOX_WIDTH;
    }

    size_t left_padding  = (UI_BOX_WIDTH - length) / 2;
    size_t right_padding = UI_BOX_WIDTH - length - left_padding;

    printf(UI_DIM "\u2502" UI_RESET " ");

    for (size_t i = 0; i < left_padding; ++i) {
        putchar(' ');
    }

    printf(UI_BOLD UI_CYAN "%s" UI_RESET, title);

    for (size_t i = 0; i < right_padding; ++i) {
        putchar(' ');
    }

    printf(" " UI_DIM "\u2502" UI_RESET "\n");
}

void ui_log(const char *color, const char *text)
{
    /* Two spaces, a dim bullet, then the coloured message. */
    printf(UI_DIM "  \u2022 " UI_RESET "%s%s" UI_RESET "\n", color, text);
}

void ui_both_lights(const char *vehicle_color,    const char *vehicle_label,
                    const char *pedestrian_color, const char *pedestrian_label)
{
    /* Two lamp indicators on one line, separated by a dim bar. */
    printf(UI_DIM "  \u2502  " UI_RESET);
    printf("VEHICLE  ");
    printf("%s\u25cf %s" UI_RESET, vehicle_color, vehicle_label);

    printf(UI_DIM "   \u2502   " UI_RESET);

    printf("WALK  ");
    printf("%s\u25cf %s" UI_RESET, pedestrian_color, pedestrian_label);
}