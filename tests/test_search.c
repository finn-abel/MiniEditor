#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "editor.h"
#include "search.h"

// Drive search_find with scripted keystrokes: stdin reads from a pipe and
// stdout is sent to /dev/null so the prompt redraw is discarded.
static void run_search_with_input(Editor *editor, const char *input, int len)
{
    int pipe_fds[2];
    int saved_stdin;
    int saved_stdout;
    int dev_null;

    assert(pipe(pipe_fds) == 0);
    assert(write(pipe_fds[1], input, (size_t) len) == len);
    close(pipe_fds[1]);

    saved_stdin = dup(STDIN_FILENO);
    assert(saved_stdin != -1);
    assert(dup2(pipe_fds[0], STDIN_FILENO) != -1);
    close(pipe_fds[0]);

    saved_stdout = dup(STDOUT_FILENO);
    assert(saved_stdout != -1);
    dev_null = open("/dev/null", O_WRONLY);
    assert(dev_null != -1);
    assert(dup2(dev_null, STDOUT_FILENO) != -1);
    close(dev_null);

    search_find(editor);

    assert(dup2(saved_stdout, STDOUT_FILENO) != -1);
    close(saved_stdout);
    assert(dup2(saved_stdin, STDIN_FILENO) != -1);
    close(saved_stdin);
}

// Build a small buffer with two "banana" matches (rows 1 and 3).
static void setup_fruit_editor(Editor *editor)
{
    editor_init(editor);
    editor->screen_rows = 5;
    editor->screen_cols = 40;
    editor_insert_row(editor, 0, "apple", 5);
    editor_insert_row(editor, 1, "banana", 6);
    editor_insert_row(editor, 2, "carrot", 6);
    editor_insert_row(editor, 3, "banana split", 12);
}

static void test_search_forward_finds_first_match(void)
{
    Editor editor;

    setup_fruit_editor(&editor);
    run_search_with_input(&editor, "banana\r", 7);
    assert(editor.cursor_y == 1);
    assert(editor.cursor_x == 0);
    editor_free(&editor);
}

static void test_search_arrow_continues_forward(void)
{
    Editor editor;

    setup_fruit_editor(&editor);
    run_search_with_input(&editor, "banana\x1b[B\r", 10);
    assert(editor.cursor_y == 3);
    assert(editor.cursor_x == 0);
    editor_free(&editor);
}

static void test_search_arrow_continues_backward(void)
{
    Editor editor;

    setup_fruit_editor(&editor);
    /* Typing matches row 1; arrow up then wraps backward to row 3. */
    run_search_with_input(&editor, "banana\x1b[A\r", 10);
    assert(editor.cursor_y == 3);
    assert(editor.cursor_x == 0);
    editor_free(&editor);
}

static void test_search_no_match_keeps_cursor(void)
{
    Editor editor;

    setup_fruit_editor(&editor);
    editor.cursor_y = 2;
    editor.cursor_x = 3;
    run_search_with_input(&editor, "zzz\r", 4);
    assert(editor.cursor_y == 2);
    assert(editor.cursor_x == 3);
    editor_free(&editor);
}

static void test_search_escape_restores_viewport(void)
{
    Editor editor;

    setup_fruit_editor(&editor);
    editor.cursor_x = 2;
    editor.cursor_y = 2;
    editor.row_offset = 1;
    editor.col_offset = 4;
    run_search_with_input(&editor, "banana\x1b", 7);
    assert(editor.cursor_y == 2);
    assert(editor.cursor_x == 2);
    assert(editor.row_offset == 1);
    assert(editor.col_offset == 4);
    editor_free(&editor);
}

int main(void)
{
    test_search_forward_finds_first_match();
    test_search_arrow_continues_forward();
    test_search_arrow_continues_backward();
    test_search_no_match_keeps_cursor();
    test_search_escape_restores_viewport();
    return 0;
}
