#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "editor.h"
#include "input.h"

static void process_key_bytes(Editor *editor, const char *keys, int len)
{
    int pipe_fds[2];
    int saved_stdin;

    assert(pipe(pipe_fds) == 0);
    assert(write(pipe_fds[1], keys, (size_t) len) == len);
    close(pipe_fds[1]);

    saved_stdin = dup(STDIN_FILENO);
    assert(saved_stdin != -1);
    assert(dup2(pipe_fds[0], STDIN_FILENO) != -1);
    close(pipe_fds[0]);

    input_process_keypress(editor);

    assert(dup2(saved_stdin, STDIN_FILENO) != -1);
    close(saved_stdin);
}

int main(void)
{
    Editor editor;

    editor_init(&editor);
    editor.screen_rows = 3;
    editor.screen_cols = 4;

    process_key_bytes(&editor, "\x1b[C", 3);
    assert(editor.cursor_x == 0);
    assert(editor.cursor_y == 0);

    process_key_bytes(&editor, "\x1b[B", 3);
    assert(editor.cursor_x == 0);
    assert(editor.cursor_y == 0);

    process_key_bytes(&editor, "\x1b[H", 3);
    assert(editor.cursor_x == 0);
    assert(editor.cursor_y == 0);

    process_key_bytes(&editor, "\x1b[F", 3);
    assert(editor.cursor_x == 0);
    assert(editor.cursor_y == 0);

    process_key_bytes(&editor, "\x1b[5~", 4);
    assert(editor.cursor_x == 0);
    assert(editor.cursor_y == 0);

    process_key_bytes(&editor, "\x1b[6~", 4);
    assert(editor.cursor_x == 0);
    assert(editor.cursor_y == 0);

    process_key_bytes(&editor, "\x11", 1);
    assert(editor.should_quit == 1);

    editor_free(&editor);

    editor_init(&editor);
    editor_insert_row(&editor, 0, "long", 4);
    editor_insert_row(&editor, 1, "x", 1);
    editor.cursor_x = 4;
    editor.cursor_y = 0;

    process_key_bytes(&editor, "\x1b[B", 3);
    assert(editor.cursor_y == 1);
    assert(editor.cursor_x == 1);

    process_key_bytes(&editor, "\x1b[C", 3);
    assert(editor.cursor_y == 1);
    assert(editor.cursor_x == 1);

    editor_free(&editor);

    editor_init(&editor);
    editor.dirty = 1;

    process_key_bytes(&editor, "\x11", 1);
    assert(editor.should_quit == 0);
    assert(editor.quit_times == 2);
    assert(strstr(editor.status_message, "2 more times") != NULL);

    process_key_bytes(&editor, "x", 1);
    assert(editor.quit_times == MiniEditor_QUIT_TIMES);

    process_key_bytes(&editor, "\x11", 1);
    process_key_bytes(&editor, "\x11", 1);
    process_key_bytes(&editor, "\x11", 1);
    assert(editor.should_quit == 1);

    editor_free(&editor);
    return 0;
}
