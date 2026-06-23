#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "editor.h"
#include "search.h"

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

int main(void)
{
    Editor editor;

    editor_init(&editor);
    editor.screen_rows = 5;
    editor.screen_cols = 40;
    editor_insert_row(&editor, 0, "apple", 5);
    editor_insert_row(&editor, 1, "banana", 6);
    editor_insert_row(&editor, 2, "carrot", 6);
    editor_insert_row(&editor, 3, "banana split", 12);

    run_search_with_input(&editor, "banana\r", 7);
    assert(editor.cursor_y == 1);
    assert(editor.cursor_x == 0);

    editor.cursor_x = 0;
    editor.cursor_y = 0;
    run_search_with_input(&editor, "banana\x1b[B\r", 10);
    assert(editor.cursor_y == 3);
    assert(editor.cursor_x == 0);

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
    return 0;
}
