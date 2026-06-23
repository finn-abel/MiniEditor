#include "row.h"

#include <stdlib.h>
#include <string.h>

// Copy raw line text into the row and build the first rendered version used by
// the screen painter.
void row_init(EditorRow *row, int index, const char *s, size_t len)
{
    row->index = index;
    row->size = (int) len;
    row->chars = malloc(len + 1);
    row->render_size = 0;
    row->render = NULL;
    row->highlight = NULL;
    row->highlight_open_comment = 0;

    if (row->chars == NULL) {
        row->size = 0;
        return;
    }

    memcpy(row->chars, s, len);
    row->chars[len] = '\0';
    row_update_render(row);
}

// Release all buffers owned by the row. Keeping fields reset makes accidental
// reuse easier to diagnose in tests.
void row_free(EditorRow *row)
{
    free(row->chars);
    free(row->render);
    free(row->highlight);

    row->chars = NULL;
    row->render = NULL;
    row->highlight = NULL;
    row->size = 0;
    row->render_size = 0;
    row->highlight_open_comment = 0;
}

// Expand raw characters into their visible terminal form. Tabs become spaces up
// to the next fixed tab stop, and highlight storage tracks rendered columns.
void row_update_render(EditorRow *row)
{
    int tabs = 0;
    int index;
    int render_index = 0;
    char *render;
    unsigned char *highlight;

    for (index = 0; index < row->size; index++) {
        if (row->chars[index] == '\t') {
            tabs++;
        }
    }

    render = malloc((size_t) row->size + (size_t) tabs *
                    (MiniEditor_TAB_STOP - 1) + 1);
    if (render == NULL) {
        return;
    }

    // Rebuild the visible row while expanding each tab to the next stop.
    for (index = 0; index < row->size; index++) {
        if (row->chars[index] == '\t') {
            render[render_index++] = ' ';
            while (render_index % MiniEditor_TAB_STOP != 0) {
                render[render_index++] = ' ';
            }
        } else {
            render[render_index++] = row->chars[index];
        }
    }
    render[render_index] = '\0';

    highlight = calloc((size_t) render_index, sizeof(*highlight));
    if (highlight == NULL && render_index > 0) {
        free(render);
        return;
    }

    free(row->render);
    free(row->highlight);
    row->render = render;
    row->render_size = render_index;
    row->highlight = highlight;
    row->highlight_open_comment = 0;
}

// Count visible columns up to a raw character position, accounting for tab
// expansion along the way.
int row_cursor_x_to_render_x(EditorRow *row, int cursor_x)
{
    int render_x = 0;
    int index;

    if (cursor_x < 0) {
        cursor_x = 0;
    }
    if (cursor_x > row->size) {
        cursor_x = row->size;
    }

    for (index = 0; index < cursor_x; index++) {
        if (row->chars[index] == '\t') {
            render_x += (MiniEditor_TAB_STOP - 1) -
                        (render_x % MiniEditor_TAB_STOP);
        }
        render_x++;
    }

    return render_x;
}

// Walk raw characters until the rendered position would pass the target column.
// That gives the closest raw cursor position for a rendered x coordinate.
int row_render_x_to_cursor_x(EditorRow *row, int render_x)
{
    int current_render_x = 0;
    int cursor_x;

    if (render_x < 0) {
        return 0;
    }

    for (cursor_x = 0; cursor_x < row->size; cursor_x++) {
        if (row->chars[cursor_x] == '\t') {
            current_render_x += (MiniEditor_TAB_STOP - 1) -
                                (current_render_x % MiniEditor_TAB_STOP);
        }
        current_render_x++;

        if (current_render_x > render_x) {
            return cursor_x;
        }
    }

    return cursor_x;
}

// Insert by growing the raw buffer, opening a one-byte gap, then refreshing the
// rendered row so tabs and future highlighting stay in sync.
void row_insert_char(EditorRow *row, int at, int c)
{
    char *chars;

    if (at < 0) {
        at = 0;
    }
    if (at > row->size) {
        at = row->size;
    }

    chars = realloc(row->chars, (size_t) row->size + 2);
    if (chars == NULL) {
        return;
    }

    row->chars = chars;
    memmove(&row->chars[at + 1], &row->chars[at],
            (size_t) (row->size - at + 1));
    row->chars[at] = (char) c;
    row->size++;
    row_update_render(row);
}

// Append text to the raw row. This is mainly used when joining two rows during
// Backspace/Delete handling.
void row_append_string(EditorRow *row, const char *s, size_t len)
{
    char *chars = realloc(row->chars, (size_t) row->size + len + 1);

    if (chars == NULL) {
        return;
    }

    row->chars = chars;
    memcpy(&row->chars[row->size], s, len);
    row->size += (int) len;
    row->chars[row->size] = '\0';
    row_update_render(row);
}

// Delete a raw character and refresh the rendered row. Deleting from an empty
// row or outside the row bounds is a no-op.
void row_delete_char(EditorRow *row, int at)
{
    if (at < 0 || at >= row->size) {
        return;
    }

    memmove(&row->chars[at], &row->chars[at + 1],
            (size_t) (row->size - at));
    row->size--;
    row_update_render(row);
}
