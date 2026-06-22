#ifndef EDITOR_H
#define EDITOR_H

#include <termios.h>

/*
 * Stores the shared state for one MiniEditor session.
 * This includes terminal state that must be restored before exit.
 */
typedef struct Editor {
    int initialized;
    int raw_mode_enabled;
    int cursor_x;
    int cursor_y;
    int screen_rows;
    int screen_cols;
    struct termios original_termios;
} Editor;

/*
 * Initializes an Editor instance into a known empty state.
 * The caller owns the Editor storage and passes it to editor_free later.
 */
void editor_init(Editor *editor);

/*
 * Releases resources owned by an Editor instance.
 * This early implementation has no dynamic resources yet.
 */
void editor_free(Editor *editor);

#endif
