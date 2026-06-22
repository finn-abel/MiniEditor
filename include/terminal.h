#ifndef TERMINAL_H
#define TERMINAL_H

#include "editor.h"

/*
 * Represents special keys decoded from terminal escape sequences.
 * Plain character keys are returned as their byte values.
 */
enum TerminalKey {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DELETE_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN
};

/*
 * Enables raw terminal mode for the active editor session.
 * The editor stores the original settings so they can be restored later.
 */
int terminal_enable_raw_mode(Editor *editor);

/*
 * Restores the terminal settings saved before raw mode was enabled.
 * It is safe to call this even if raw mode was not successfully enabled.
 */
void terminal_disable_raw_mode(Editor *editor);

/*
 * Clears the terminal display and moves the cursor back to the top-left.
 * This is used when leaving the editor so the shell returns cleanly.
 */
void terminal_clear_screen(void);

/*
 * Reads one key from standard input while the terminal is in raw mode.
 * Returns the byte read as an unsigned char value, or -1 on failure.
 */
int terminal_read_key(void);

/*
 * Gets the current terminal window size in rows and columns.
 * Returns 0 on success and -1 if the size cannot be detected.
 */
int terminal_get_window_size(int *rows, int *cols);

#endif
