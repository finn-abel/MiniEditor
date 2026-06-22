#include "render.h"

#include "abuf.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void render_draw_file_row(Editor *editor, AppendBuffer *ab, int row_index)
{
    int len = editor->rows[row_index].render_size;

    if (len > editor->screen_cols) {
        len = editor->screen_cols;
    }

    abuf_append(ab, editor->rows[row_index].render, len);
}

// Draw each visible editor row, falling back to the placeholder tilde for empty
// lines below the current buffer.
static void render_draw_rows(Editor *editor, AppendBuffer *ab)
{
    int y;

    for (y = 0; y < editor->screen_rows; y++) {
        if (y < editor->row_count) {
            render_draw_file_row(editor, ab, y);
        } else if (editor->row_count == 0 && y == editor->screen_rows / 3) {
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

static void render_draw_status_bar(Editor *editor, AppendBuffer *ab)
{
    char status[120];
    const char *dirty_state = editor->dirty ? "modified" : "clean";
    int len;
    int line_info_len;
    int spaces;

    len = snprintf(status, sizeof(status), "%s - %d lines",
                   editor->filename != NULL ? editor->filename : "[No Name]",
                   editor->row_count);
    if (len < 0) {
        len = 0;
    }
    if (len > editor->screen_cols) {
        len = editor->screen_cols;
    }

    abuf_append(ab, "\x1b[7m", 4);
    abuf_append(ab, status, len);

    line_info_len = (int) strlen(dirty_state);
    spaces = editor->screen_cols - len - line_info_len;
    while (spaces > 0) {
        abuf_append(ab, " ", 1);
        spaces--;
    }

    if (line_info_len <= editor->screen_cols - len) {
        abuf_append(ab, dirty_state, line_info_len);
    }

    abuf_append(ab, "\x1b[m", 3);
    abuf_append(ab, "\r\n", 2);
}

static void render_draw_message_bar(Editor *editor, AppendBuffer *ab)
{
    const char *message = "HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = search";
    int message_len;

    abuf_append(ab, "\x1b[K", 3);

    if (editor->status_message[0] != '\0' &&
        time(NULL) - editor->status_message_time < 5) {
        message = editor->status_message;
    }

    message_len = (int) strlen(message);
    if (message_len > editor->screen_cols) {
        message_len = editor->screen_cols;
    }

    abuf_append(ab, message, message_len);
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
    abuf_append(&ab, "\r\n", 2);
    render_draw_status_bar(editor, &ab);
    render_draw_message_bar(editor, &ab);

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
