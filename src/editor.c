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
    editor->render_x = 0;
    editor->row_offset = 0;
    editor->col_offset = 0;
    editor->line_number_width = 1;
    editor->screen_rows = 22;
    editor->screen_cols = 80;
    editor->row_count = 0;
    editor->dirty = 0;
    editor->filename = NULL;
    editor->status_message[0] = '\0';
    editor->status_message_time = 0;
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
    free(editor->filename);

    editor->initialized = 0;
    editor->raw_mode_enabled = 0;
    editor->should_quit = 0;
    editor->screen_needs_clear = 0;
    editor->render_x = 0;
    editor->row_offset = 0;
    editor->col_offset = 0;
    editor->line_number_width = 1;
    editor->row_count = 0;
    editor->dirty = 0;
    editor->filename = NULL;
    editor->status_message[0] = '\0';
    editor->status_message_time = 0;
    editor->rows = NULL;
}

void editor_scroll(Editor *editor)
{
    int text_cols = editor->screen_cols - editor->line_number_width - 1;

    editor->render_x = 0;
    if (editor->cursor_y >= 0 && editor->cursor_y < editor->row_count) {
        editor->render_x =
            row_cursor_x_to_render_x(&editor->rows[editor->cursor_y],
                                     editor->cursor_x);
    }

    if (editor->cursor_y < editor->row_offset) {
        editor->row_offset = editor->cursor_y;
    }
    if (editor->cursor_y >= editor->row_offset + editor->screen_rows) {
        editor->row_offset = editor->cursor_y - editor->screen_rows + 1;
    }

    if (text_cols < 1) {
        text_cols = 1;
    }

    if (editor->render_x < editor->col_offset) {
        editor->col_offset = editor->render_x;
    }
    if (editor->render_x >= editor->col_offset + text_cols) {
        editor->col_offset = editor->render_x - text_cols + 1;
    }
}
