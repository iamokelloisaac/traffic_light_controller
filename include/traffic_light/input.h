/**
* @file input.h
 * @brief Non-blocking keystroke polling.
 *
 * "Non-blocking" means the functions here return immediately,
 * whether or not a key was pressed. The state machine calls them
 * in a loop to check for new input without pausing execution.
 */

#ifndef INPUT_H
#define INPUT_H

/**
 * @brief The kinds of input events the state machine reacts to.
 *
 * Using an enum instead of raw integers means the code reads as
 * English: `if (event == INPUT_STOP)` is clearer than `if (x == 1)`.
 */
typedef enum {
    INPUT_NONE = 0,   /* No key was pressed. */
    INPUT_BUTTON,     /* Any key except 'q' or 'Q'. */
    INPUT_STOP        /* 'q' or 'Q'. */
} InputEvent;

/**
 * @brief Polls stdin once, non-blocking.
 *
 * If a key is available, this function returns it classified as
 * an InputEvent. If the user typed several keys at once, the
 * extra ones are discarded so a single burst cannot produce
 * multiple events.
 *
 * @return The event, or INPUT_NONE if no key was ready.
 */
InputEvent input_poll();

/**
 * @brief Throws away everything currently readable on stdin.
 *
 * Useful when you want to clear stale input before entering a
 * fresh loop.
 */
void input_clear_buffer();

#endif /* INPUT_H */