#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "editor.h"
#include "render.h"
#include "row.h"
#include "status.h"
#include "syntax.h"

// Capture one rendered frame by redirecting stdout into a pipe.
static int capture_render_output(Editor *editor, char *buffer, int buffer_size)
{
    int pipe_fds[2];
    int saved_stdout;
    ssize_t bytes_read;

    assert(pipe(pipe_fds) == 0);
    saved_stdout = dup(STDOUT_FILENO);
    assert(saved_stdout != -1);
    assert(dup2(pipe_fds[1], STDOUT_FILENO) != -1);
    close(pipe_fds[1]);

    render_refresh_screen(editor);

    assert(dup2(saved_stdout, STDOUT_FILENO) != -1);
    close(saved_stdout);

    bytes_read = read(pipe_fds[0], buffer, (size_t) buffer_size - 1);
    close(pipe_fds[0]);
    assert(bytes_read >= 0);

    buffer[bytes_read] = '\0';
    return (int) bytes_read;
}

static void test_render_empty_buffer(void)
{
    Editor editor;
    char output[1024];

    editor_init(&editor);
    editor.screen_rows = 5;
    editor.screen_cols = 80;
    editor.cursor_x = 2;
    editor.cursor_y = 3;

    assert(capture_render_output(&editor, output, (int) sizeof(output)) > 0);
    assert(strstr(output, "\x1b[?25l") != NULL);
    assert(strstr(output, "\x1b[H") != NULL);
    assert(strstr(output, "\x1b[2J") != NULL);
    assert(strstr(output, "MiniEditor") != NULL);
    assert(strstr(output, "\x1b[4;3H") != NULL);
    assert(strstr(output, "\x1b[?25h") != NULL);
    assert(strstr(output, "[No Name] - 0 lines") != NULL);
    assert(strstr(output, "clean") != NULL);
    assert(strstr(output, "HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = search") != NULL);

    editor_free(&editor);
}

static void test_render_skips_clear_on_second_frame(void)
{
    Editor editor;
    char output[1024];

    editor_init(&editor);
    editor.screen_rows = 5;
    editor.screen_cols = 80;

    assert(capture_render_output(&editor, output, (int) sizeof(output)) > 0);
    assert(capture_render_output(&editor, output, (int) sizeof(output)) > 0);
    assert(strstr(output, "\x1b[H") != NULL);
    assert(strstr(output, "\x1b[2J") == NULL);

    editor_free(&editor);
}

static void test_render_shows_rows_status_and_cursor(void)
{
    Editor editor;
    char output[1024];

    editor_init(&editor);
    editor.screen_rows = 5;
    editor.screen_cols = 80;
    editor_insert_row(&editor, 0, "hello", 5);
    status_set(&editor, "Loaded %d line", editor.row_count);

    assert(capture_render_output(&editor, output, (int) sizeof(output)) > 0);
    assert(strstr(output, "hello") != NULL);
    assert(strstr(output, "modified") != NULL);
    assert(strstr(output, "Loaded 1 line") != NULL);

    editor.cursor_x = 0;
    editor.cursor_y = 0;
    editor.row_offset = 0;
    editor.col_offset = 0;
    assert(capture_render_output(&editor, output, (int) sizeof(output)) > 0);
    assert(strstr(output, "1 hello") != NULL);
    assert(strstr(output, "\x1b[1;3H") != NULL);

    editor_free(&editor);
}

static void test_render_emits_syntax_colors(void)
{
    Editor editor;
    char output[1024];

    editor_init(&editor);
    editor.screen_rows = 5;
    editor.screen_cols = 80;
    editor.filename = malloc(strlen("snippet.c") + 1);
    assert(editor.filename != NULL);
    strcpy(editor.filename, "snippet.c");
    editor_insert_row(&editor, 0, "int x;", 6);
    syntax_select(&editor);

    assert(capture_render_output(&editor, output, (int) sizeof(output)) > 0);
    /* "int" is a type keyword (group 2 -> color 32). */
    assert(strstr(output, "\x1b[32m") != NULL);
    /* Color runs are reset back to normal afterward. */
    assert(strstr(output, "\x1b[m") != NULL);

    editor_free(&editor);
}

static void test_render_emits_match_highlight(void)
{
    Editor editor;
    char output[1024];

    editor_init(&editor);
    editor.screen_rows = 5;
    editor.screen_cols = 80;
    editor_insert_row(&editor, 0, "needle", 6);
    /* Simulate a search match marked on the first character. */
    editor.rows[0].highlight[0] = HL_MATCH;

    assert(capture_render_output(&editor, output, (int) sizeof(output)) > 0);
    assert(strstr(output, "\x1b[34;7m") != NULL);

    editor_free(&editor);
}

int main(void)
{
    test_render_empty_buffer();
    test_render_skips_clear_on_second_frame();
    test_render_shows_rows_status_and_cursor();
    test_render_emits_syntax_colors();
    test_render_emits_match_highlight();
    return 0;
}
