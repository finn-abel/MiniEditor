#ifndef TERMINAL_H
#define TERMINAL_H

#include "editor.h"

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
