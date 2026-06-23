#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "editor.h"
#include "terminal.h"

// Capture terminal_clear_screen output by redirecting stdout into a pipe.
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

// Feed raw bytes to terminal_read_key from a pipe and return the decoded key.
static int read_key_from_bytes(const char *bytes, int len)
{
    int pipe_fds[2];
    int saved_stdin;
    int key;

    assert(pipe(pipe_fds) == 0);
    assert(write(pipe_fds[1], bytes, (size_t) len) == len);
    close(pipe_fds[1]);

    saved_stdin = dup(STDIN_FILENO);
    assert(saved_stdin != -1);
    assert(dup2(pipe_fds[0], STDIN_FILENO) != -1);
    close(pipe_fds[0]);

    key = terminal_read_key();

    assert(dup2(saved_stdin, STDIN_FILENO) != -1);
    close(saved_stdin);
    return key;
}

static void test_disable_raw_mode_is_safe_when_off(void)
{
    Editor editor;

    editor_init(&editor);
    terminal_disable_raw_mode(&editor);
    assert(editor.raw_mode_enabled == 0);
    editor_free(&editor);
}

static void test_clear_screen_output(void)
{
    char output[32];

    assert(capture_clear_output(output, (int) sizeof(output)) > 0);
    assert(strcmp(output, "\x1b[2J\x1b[H") == 0);
}

static void test_read_key_plain_and_bare_escape(void)
{
    assert(read_key_from_bytes("a", 1) == 'a');
    /* A lone Escape with nothing following decodes back to Escape. */
    assert(read_key_from_bytes("\x1b", 1) == '\x1b');
}

static void test_read_key_decodes_escape_sequences(void)
{
    assert(read_key_from_bytes("\x1b[A", 3) == ARROW_UP);
    assert(read_key_from_bytes("\x1b[B", 3) == ARROW_DOWN);
    assert(read_key_from_bytes("\x1b[C", 3) == ARROW_RIGHT);
    assert(read_key_from_bytes("\x1b[D", 3) == ARROW_LEFT);
    assert(read_key_from_bytes("\x1b[H", 3) == HOME_KEY);
    assert(read_key_from_bytes("\x1b[F", 3) == END_KEY);
    assert(read_key_from_bytes("\x1bOH", 3) == HOME_KEY);
    assert(read_key_from_bytes("\x1bOF", 3) == END_KEY);

    assert(read_key_from_bytes("\x1b[1~", 4) == HOME_KEY);
    assert(read_key_from_bytes("\x1b[3~", 4) == DELETE_KEY);
    assert(read_key_from_bytes("\x1b[4~", 4) == END_KEY);
    assert(read_key_from_bytes("\x1b[5~", 4) == PAGE_UP);
    assert(read_key_from_bytes("\x1b[6~", 4) == PAGE_DOWN);
    assert(read_key_from_bytes("\x1b[7~", 4) == HOME_KEY);
    assert(read_key_from_bytes("\x1b[8~", 4) == END_KEY);
}

static void test_get_window_size(void)
{
    int rows = -1;
    int cols = -1;
    int result = terminal_get_window_size(&rows, &cols);

    if (result == 0) {
        assert(rows > 0);
        assert(cols > 0);
    } else {
        assert(result == -1);
        assert(rows == -1);
        assert(cols == -1);
    }
}

int main(void)
{
    test_disable_raw_mode_is_safe_when_off();
    test_clear_screen_output();
    test_read_key_plain_and_bare_escape();
    test_read_key_decodes_escape_sequences();
    test_get_window_size();
    return 0;
}
