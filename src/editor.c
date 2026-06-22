#include "editor.h"

#include <stdlib.h>

// Set defaults that let small unit tests use an Editor without requiring a
// real terminal. main.c replaces the screen size after raw mode is enabled.
void editor_init(Editor *editor)
{
    editor->initialized = 1;
    editor->raw_mode_enabled = 0;
    editor->should_quit = 0;
    editor->screen_needs_clear = 1;
    editor->cursor_x = 0;
    editor->cursor_y = 0;
    editor->screen_rows = 22;
    editor->screen_cols = 80;
    editor->row_count = 0;
    editor->dirty = 0;
    editor->rows = NULL;
}

// Release all rows owned by this editor before marking the session inactive.
void editor_free(Editor *editor)
{
    int index;

    for (index = 0; index < editor->row_count; index++) {
        row_free(&editor->rows[index]);
    }
    free(editor->rows);

    editor->initialized = 0;
    editor->raw_mode_enabled = 0;
    editor->should_quit = 0;
    editor->screen_needs_clear = 0;
    editor->row_count = 0;
    editor->dirty = 0;
    editor->rows = NULL;
}
