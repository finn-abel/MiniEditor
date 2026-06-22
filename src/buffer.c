#include "buffer.h"

#include <stdlib.h>
#include <string.h>

// After a shift, keep row indexes matching their physical array positions.
static void buffer_update_row_indexes(Editor *editor, int start)
{
    int index;

    for (index = start; index < editor->row_count; index++) {
        editor->rows[index].index = index;
    }
}

// Insert one initialized row and shift later rows down. Invalid insertion points
// are ignored so callers can probe without corrupting the buffer.
void editor_insert_row(Editor *editor, int at, const char *s, size_t len)
{
    EditorRow *new_rows;

    if (at < 0 || at > editor->row_count) {
        return;
    }

    new_rows = realloc(editor->rows,
                       sizeof(*editor->rows) * (size_t) (editor->row_count + 1));
    if (new_rows == NULL) {
        return;
    }

    editor->rows = new_rows;
    if (at < editor->row_count) {
        memmove(&editor->rows[at + 1], &editor->rows[at],
                sizeof(*editor->rows) * (size_t) (editor->row_count - at));
    }

    row_init(&editor->rows[at], at, s, len);
    editor->row_count++;
    buffer_update_row_indexes(editor, at + 1);
    editor->dirty++;
}

// Delete a row, close the gap, and mark the editor dirty for future save
// prompts. Invalid indexes leave the buffer untouched.
void editor_delete_row(Editor *editor, int at)
{
    if (at < 0 || at >= editor->row_count) {
        return;
    }

    row_free(&editor->rows[at]);

    if (at < editor->row_count - 1) {
        memmove(&editor->rows[at], &editor->rows[at + 1],
                sizeof(*editor->rows) * (size_t) (editor->row_count - at - 1));
    }

    editor->row_count--;
    buffer_update_row_indexes(editor, at);
    editor->dirty++;
}
