#include "search.h"

#include "prompt.h"
#include "terminal.h"

#include <stdlib.h>
#include <string.h>

static int search_last_match = -1;

// Search highlighting is temporary. Clear every row before applying the active
// match so stale highlights do not linger while the query changes.
static void search_clear_matches(Editor *editor)
{
    int index;

    for (index = 0; index < editor->row_count; index++) {
        if (editor->rows[index].highlight != NULL) {
            memset(editor->rows[index].highlight, HL_NORMAL,
                   (size_t) editor->rows[index].render_size);
        }
    }
}

// Return the raw cursor position of the first substring match in one row.
// The caller converts that raw position to rendered columns for highlighting.
static int search_find_in_row(EditorRow *row, const char *query)
{
    char *match = strstr(row->chars, query);

    if (match == NULL) {
        return -1;
    }

    return (int) (match - row->chars);
}

// Mark the current match in render-space. This keeps highlighting aligned with
// tabs because the render buffer may be wider than the raw row text.
static void search_highlight_match(Editor *editor, int row_index, int cursor_x,
                                   int query_len)
{
    int render_x;
    int highlight_len;
    EditorRow *row = &editor->rows[row_index];

    render_x = row_cursor_x_to_render_x(row, cursor_x);
    highlight_len = query_len;
    if (render_x + highlight_len > row->render_size) {
        highlight_len = row->render_size - render_x;
    }

    if (highlight_len > 0) {
        memset(&row->highlight[render_x], HL_MATCH, (size_t) highlight_len);
    }
}

// Prompt callback used for live search. Typing a query starts from the top, and
// arrow keys continue searching forward or backward from the last match.
static void search_callback(Editor *editor, const char *query, int key)
{
    int direction = 1;
    int current;
    int index;
    int query_len = (int) strlen(query);

    search_clear_matches(editor);

    if (query_len == 0) {
        search_last_match = -1;
        return;
    }

    if (key == ARROW_UP || key == ARROW_LEFT) {
        direction = -1;
    } else if (key == ARROW_DOWN || key == ARROW_RIGHT) {
        direction = 1;
    } else {
        search_last_match = -1;
    }

    current = search_last_match;
    for (index = 0; index < editor->row_count; index++) {
        int cursor_x;

        current += direction;
        if (current == -1) {
            current = editor->row_count - 1;
        } else if (current == editor->row_count) {
            current = 0;
        }

        cursor_x = search_find_in_row(&editor->rows[current], query);
        if (cursor_x != -1) {
            search_last_match = current;
            editor->cursor_y = current;
            editor->cursor_x = cursor_x;
            // Force the next render to scroll the match into view.
            editor->row_offset = editor->row_count;
            search_highlight_match(editor, current, cursor_x, query_len);
            return;
        }
    }
}

void search_find(Editor *editor)
{
    int saved_cursor_x = editor->cursor_x;
    int saved_cursor_y = editor->cursor_y;
    int saved_row_offset = editor->row_offset;
    int saved_col_offset = editor->col_offset;
    char *query;

    search_last_match = -1;
    query = prompt_read_with_callback(editor, "Search: %s", search_callback);

    // Esc cancels search and restores the viewport exactly as it was before the
    // prompt opened. Enter keeps the cursor at the currently selected match.
    if (query == NULL) {
        editor->cursor_x = saved_cursor_x;
        editor->cursor_y = saved_cursor_y;
        editor->row_offset = saved_row_offset;
        editor->col_offset = saved_col_offset;
    } else {
        free(query);
    }

    search_clear_matches(editor);
}
