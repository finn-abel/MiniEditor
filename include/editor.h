#ifndef EDITOR_H
#define EDITOR_H

#include <time.h>
#include <termios.h>

#include "row.h"

#define MiniEditor_QUIT_TIMES 3

/*
 * Stores the shared state for one MiniEditor session.
 * This includes terminal state, cursor position, and owned text rows.
 */
typedef struct Editor {
    int initialized;
    int raw_mode_enabled;
    int should_quit;
    int quit_times;
    int screen_needs_clear;
    int cursor_x;
    int cursor_y;
    int render_x;
    int row_offset;
    int col_offset;
    int line_number_width;
    int screen_rows;
    int screen_cols;
    int row_count;
    int dirty;
    char *filename;
    char status_message[80];
    time_t status_message_time;
    EditorRow *rows;
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

/*
 * Updates row and column scroll offsets so the cursor remains visible.
 * The cursor's raw x position is converted to rendered x for tab-aware rows.
 */
void editor_scroll(Editor *editor);

#endif
