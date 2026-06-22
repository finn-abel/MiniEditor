#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "editor.h"
#include "render.h"

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

int main(void)
{
    Editor editor;
    char output[1024];
    int len;

    editor_init(&editor);
    editor.screen_rows = 5;
    editor.screen_cols = 20;
    editor.cursor_x = 2;
    editor.cursor_y = 3;

    len = capture_render_output(&editor, output, (int) sizeof(output));
    assert(len > 0);

    assert(strstr(output, "\x1b[?25l") != NULL);
    assert(strstr(output, "\x1b[H") != NULL);
    assert(strstr(output, "\x1b[2J") != NULL);
    assert(strstr(output, "MiniEditor") != NULL);
    assert(strstr(output, "\x1b[4;3H") != NULL);
    assert(strstr(output, "\x1b[?25h") != NULL);

    len = capture_render_output(&editor, output, (int) sizeof(output));
    assert(len > 0);
    assert(strstr(output, "\x1b[H") != NULL);
    assert(strstr(output, "\x1b[2J") == NULL);

    editor_free(&editor);
    return 0;
}
