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

void editor_insert_char(Editor *editor, int c)
{
    if (editor->cursor_y == editor->row_count) {
        int dirty_before = editor->dirty;

        editor_insert_row(editor, editor->row_count, "", 0);
        if (editor->cursor_y != editor->row_count - 1) {
            return;
        }
        editor->dirty = dirty_before;
    }

    if (editor->cursor_y < 0 || editor->cursor_y >= editor->row_count) {
        return;
    }

    row_insert_char(&editor->rows[editor->cursor_y], editor->cursor_x, c);
    editor->cursor_x++;
    editor->dirty++;
}

void editor_insert_newline(Editor *editor)
{
    EditorRow *row;

    if (editor->cursor_y < 0) {
        return;
    }

    if (editor->cursor_y >= editor->row_count) {
        editor_insert_row(editor, editor->row_count, "", 0);
        editor->cursor_y = editor->row_count - 1;
        editor->cursor_x = 0;
        return;
    }

    row = &editor->rows[editor->cursor_y];
    if (editor->cursor_x <= 0) {
        editor_insert_row(editor, editor->cursor_y, "", 0);
    } else if (editor->cursor_x >= row->size) {
        editor_insert_row(editor, editor->cursor_y + 1, "", 0);
        editor->cursor_y++;
    } else {
        editor_insert_row(editor, editor->cursor_y + 1,
                          &row->chars[editor->cursor_x],
                          (size_t) (row->size - editor->cursor_x));
        row = &editor->rows[editor->cursor_y];
        row->size = editor->cursor_x;
        row->chars[row->size] = '\0';
        row_update_render(row);
        editor->cursor_y++;
    }

    editor->cursor_x = 0;
}

void editor_delete_char(Editor *editor)
{
    EditorRow *row;
    int previous_size;

    if (editor->cursor_y < 0 || editor->cursor_y >= editor->row_count) {
        return;
    }

    if (editor->cursor_x == 0 && editor->cursor_y == 0) {
        return;
    }

    row = &editor->rows[editor->cursor_y];
    if (editor->cursor_x > 0) {
        row_delete_char(row, editor->cursor_x - 1);
        editor->cursor_x--;
        editor->dirty++;
        return;
    }

    previous_size = editor->rows[editor->cursor_y - 1].size;
    row_append_string(&editor->rows[editor->cursor_y - 1], row->chars,
                      (size_t) row->size);
    editor_delete_row(editor, editor->cursor_y);
    editor->cursor_y--;
    editor->cursor_x = previous_size;
}

char *editor_rows_to_string(Editor *editor, int *buf_len)
{
    char *buf;
    char *p;
    int total_len = 0;
    int index;

    for (index = 0; index < editor->row_count; index++) {
        total_len += editor->rows[index].size;
        if (index < editor->row_count - 1) {
            total_len++;
        }
    }

    buf = malloc((size_t) total_len + 1);
    if (buf == NULL) {
        *buf_len = 0;
        return NULL;
    }

    p = buf;
    for (index = 0; index < editor->row_count; index++) {
        memcpy(p, editor->rows[index].chars, (size_t) editor->rows[index].size);
        p += editor->rows[index].size;

        if (index < editor->row_count - 1) {
            *p = '\n';
            p++;
        }
    }

    *p = '\0';
    *buf_len = total_len;
    return buf;
}
