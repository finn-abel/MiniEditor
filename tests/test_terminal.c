#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "editor.h"
#include "terminal.h"

static int capture_clear_output(char *buffer, int buffer_size)
{
    int pipe_fds[2];
    int saved_stdout;
    ssize_t bytes_read;

    assert(pipe(pipe_fds) == 0);
    saved_stdout = dup(STDOUT_FILENO);
    assert(saved_stdout != -1);
    assert(dup2(pipe_fds[1], STDOUT_FILENO) != -1);
    close(pipe_fds[1]);

    terminal_clear_screen();

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
    char output[32];
    int rows = -1;
    int cols = -1;
    int result;

    editor_init(&editor);
    terminal_disable_raw_mode(&editor);
    assert(editor.raw_mode_enabled == 0);

    assert(capture_clear_output(output, (int) sizeof(output)) > 0);
    assert(strcmp(output, "\x1b[2J\x1b[H") == 0);

    result = terminal_get_window_size(&rows, &cols);
    if (result == 0) {
        assert(rows > 0);
        assert(cols > 0);
    } else {
        assert(result == -1);
        assert(rows == -1);
        assert(cols == -1);
    }

    editor_free(&editor);
    return 0;
}
