#include "render.h"

#include "abuf.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Draw the blank editor area. The welcome line is centered horizontally and
// placed near the top third so it remains visible on small terminals.
static void render_draw_rows(Editor *editor, AppendBuffer *ab)
{
    int y;

    for (y = 0; y < editor->screen_rows; y++) {
        if (y == editor->screen_rows / 3) {
            const char *welcome = "MiniEditor";
            int welcome_len = (int) strlen(welcome);
            int padding = (editor->screen_cols - welcome_len) / 2;

            if (padding > 0) {
                abuf_append(ab, "~", 1);
                padding--;
            }

            while (padding > 0) {
                abuf_append(ab, " ", 1);
                padding--;
            }

            abuf_append(ab, welcome, welcome_len);
        } else {
            abuf_append(ab, "~", 1);
        }

        abuf_append(ab, "\x1b[K", 3);
        if (y < editor->screen_rows - 1) {
            abuf_append(ab, "\r\n", 2);
        }
    }
}

// Build a whole terminal frame in memory first, then write it once. This keeps
// redraw behavior predictable as more UI elements are added.
void render_refresh_screen(Editor *editor)
{
    char cursor_position[32];
    int cursor_position_len;
    AppendBuffer ab = {NULL, 0};

    abuf_append(&ab, "\x1b[?25l", 6);
    abuf_append(&ab, "\x1b[H", 3);

    if (editor->screen_needs_clear) {
        abuf_append(&ab, "\x1b[2J", 4);
        editor->screen_needs_clear = 0;
    }

    render_draw_rows(editor, &ab);

    cursor_position_len = snprintf(cursor_position, sizeof(cursor_position),
                                   "\x1b[%d;%dH", editor->cursor_y + 1,
                                   editor->cursor_x + 1);
    if (cursor_position_len > 0 &&
        cursor_position_len < (int) sizeof(cursor_position)) {
        abuf_append(&ab, cursor_position, cursor_position_len);
    }

    abuf_append(&ab, "\x1b[?25h", 6);
    (void) write(STDOUT_FILENO, ab.data, (size_t) ab.len);
    abuf_free(&ab);
}
