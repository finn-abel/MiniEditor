#include "render.h"

#include "abuf.h"
#include "syntax.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void render_draw_file_row(Editor *editor, AppendBuffer *ab, int row_index)
{
    int index;
    int len = editor->rows[row_index].render_size - editor->col_offset;
    int text_cols = editor->screen_cols - editor->line_number_width - 1;
    int current_highlight = HL_NORMAL;

    if (text_cols < 0) {
        text_cols = 0;
    }

    if (len < 0) {
        len = 0;
    }
    if (len > text_cols) {
        len = text_cols;
    }

    for (index = 0; index < len; index++) {
        int render_index = index + editor->col_offset;
        int highlight = editor->rows[row_index].highlight[render_index];

        if (highlight != current_highlight) {
            if (highlight == HL_MATCH) {
                abuf_append(ab, "\x1b[34;7m", 7);
            } else if (highlight == HL_NORMAL) {
                abuf_append(ab, "\x1b[m", 3);
            } else {
                char color[16];
                int color_len = snprintf(color, sizeof(color), "\x1b[%dm",
                                         syntax_color(highlight));

                if (color_len > 0 && color_len < (int) sizeof(color)) {
                    abuf_append(ab, color, color_len);
                }
            }
            current_highlight = highlight;
        }

        abuf_append(ab, &editor->rows[row_index].render[render_index], 1);
    }

    if (current_highlight != HL_NORMAL) {
        abuf_append(ab, "\x1b[m", 3);
    }
}

static int render_line_number_width(Editor *editor)
{
    int rows = editor->row_count;
    int width = 1;

    while (rows >= 10) {
        rows /= 10;
        width++;
    }

    return width;
}

static void render_draw_gutter(Editor *editor, AppendBuffer *ab, int file_row)
{
    char gutter[32];
    int len;

    if (file_row < editor->row_count) {
        len = snprintf(gutter, sizeof(gutter), "%*d ",
                       editor->line_number_width, file_row + 1);
    } else {
        len = snprintf(gutter, sizeof(gutter), "%*s ",
                       editor->line_number_width, "");
    }

    if (len > 0) {
        abuf_append(ab, gutter, len);
    }
}

// Draw each visible editor row, falling back to the placeholder tilde for empty
// lines below the current buffer.
static void render_draw_rows(Editor *editor, AppendBuffer *ab)
{
    int y;

    for (y = 0; y < editor->screen_rows; y++) {
        int file_row = y + editor->row_offset;

        render_draw_gutter(editor, ab, file_row);

        if (file_row < editor->row_count) {
            render_draw_file_row(editor, ab, file_row);
        } else if (editor->row_count == 0 && y == editor->screen_rows / 3) {
            const char *welcome = "MiniEditor";
            int welcome_len = (int) strlen(welcome);
            int text_cols = editor->screen_cols - editor->line_number_width - 1;
            int padding = (text_cols - welcome_len) / 2;

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

    len = snprintf(status, sizeof(status), "%s - %d lines | %s",
                   editor->filename != NULL ? editor->filename : "[No Name]",
                   editor->row_count,
                   editor->syntax != NULL ? editor->syntax->filetype : "no ft");
    if (len < 0) {
        len = 0;
    }
    // snprintf returns the length it *would* have written, so clamp to the
    // bytes actually stored before clamping to the screen width. Otherwise a
    // long filename on a wide terminal makes abuf_append read past `status`.
    if (len >= (int) sizeof(status)) {
        len = (int) sizeof(status) - 1;
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
    int screen_cursor_x;
    int screen_cursor_y;
    AppendBuffer ab = {NULL, 0};

    editor->line_number_width = render_line_number_width(editor);
    editor_scroll(editor);

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

    screen_cursor_x = editor->render_x - editor->col_offset +
                      editor->line_number_width + 1;
    screen_cursor_y = editor->cursor_y - editor->row_offset;
    if (screen_cursor_x < editor->line_number_width + 1) {
        screen_cursor_x = editor->line_number_width + 1;
    }
    if (screen_cursor_y < 0) {
        screen_cursor_y = 0;
    }

    cursor_position_len = snprintf(cursor_position, sizeof(cursor_position),
                                   "\x1b[%d;%dH", screen_cursor_y + 1,
                                   screen_cursor_x + 1);
    if (cursor_position_len > 0 &&
        cursor_position_len < (int) sizeof(cursor_position)) {
        abuf_append(&ab, cursor_position, cursor_position_len);
    }

    abuf_append(&ab, "\x1b[?25h", 6);
    (void) write(STDOUT_FILENO, ab.data, (size_t) ab.len);
    abuf_free(&ab);
}
