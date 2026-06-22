#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "editor.h"
#include "prompt.h"

static char *run_prompt_with_input(Editor *editor, const char *input, int len)
{
    int pipe_fds[2];
    int saved_stdin;
    int saved_stdout;
    int dev_null;
    char *result;

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

    result = prompt_read(editor, "Save as: %s");

    assert(dup2(saved_stdout, STDOUT_FILENO) != -1);
    close(saved_stdout);
    assert(dup2(saved_stdin, STDIN_FILENO) != -1);
    close(saved_stdin);
    return result;
}

int main(void)
{
    Editor editor;
    char *result;

    editor_init(&editor);
    editor.screen_rows = 3;
    editor.screen_cols = 40;

    result = run_prompt_with_input(&editor, "note.tx\bxt\r", 11);
    assert(result != NULL);
    assert(strcmp(result, "note.txt") == 0);
    assert(editor.status_message[0] == '\0');
    free(result);

    result = run_prompt_with_input(&editor, "cancel\x1b", 7);
    assert(result == NULL);
    assert(editor.status_message[0] == '\0');

    editor_free(&editor);
    return 0;
}
