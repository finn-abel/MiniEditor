#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "editor.h"
#include "fileio.h"

int main(void)
{
    Editor editor;
    const char *path = "/tmp/minieditor_fileio_test.txt";
    const char *save_path = "/tmp/minieditor_save_test.txt";
    const char *prompt_save_path = "/tmp/minieditor_prompt_save_test.txt";
    FILE *fp;
    char saved[64];
    size_t saved_len;

    fp = fopen(path, "w");
    assert(fp != NULL);
    fputs("#include <stdio.h>\n\nint main(void) {\r\n    return 0;\n}", fp);
    fclose(fp);

    editor_init(&editor);
    fileio_open(&editor, path);

    assert(editor.filename != NULL);
    assert(strcmp(editor.filename, path) == 0);
    assert(editor.row_count == 5);
    assert(editor.dirty == 0);
    assert(strcmp(editor.rows[0].chars, "#include <stdio.h>") == 0);
    assert(strcmp(editor.rows[1].chars, "") == 0);
    assert(strcmp(editor.rows[2].chars, "int main(void) {") == 0);
    assert(strcmp(editor.rows[3].chars, "    return 0;") == 0);
    assert(strcmp(editor.rows[4].chars, "}") == 0);

    editor_free(&editor);

    editor_init(&editor);
    free(editor.filename);
    editor.filename = malloc(strlen(save_path) + 1);
    assert(editor.filename != NULL);
    strcpy(editor.filename, save_path);
    editor_insert_row(&editor, 0, "hello", 5);
    editor_insert_row(&editor, 1, "world", 5);
    editor.dirty = 1;

    assert(fileio_save(&editor) == 0);
    assert(editor.dirty == 0);
    assert(strstr(editor.status_message, "11 bytes written") != NULL);

    fp = fopen(save_path, "rb");
    assert(fp != NULL);
    saved_len = fread(saved, 1, sizeof(saved) - 1, fp);
    fclose(fp);
    saved[saved_len] = '\0';
    assert(strcmp(saved, "hello\nworld") == 0);
    editor_free(&editor);

    editor_init(&editor);
    editor_insert_row(&editor, 0, "prompted", 8);
    editor.dirty = 1;
    {
        int pipe_fds[2];
        int saved_stdin;
        int saved_stdout;
        int dev_null;

        assert(pipe(pipe_fds) == 0);
        assert(write(pipe_fds[1], prompt_save_path, strlen(prompt_save_path)) ==
               (ssize_t) strlen(prompt_save_path));
        assert(write(pipe_fds[1], "\r", 1) == 1);
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

        assert(fileio_save(&editor) == 0);

        assert(dup2(saved_stdout, STDOUT_FILENO) != -1);
        close(saved_stdout);
        assert(dup2(saved_stdin, STDIN_FILENO) != -1);
        close(saved_stdin);
    }
    assert(editor.filename != NULL);
    assert(strcmp(editor.filename, prompt_save_path) == 0);
    assert(editor.dirty == 0);
    fp = fopen(prompt_save_path, "rb");
    assert(fp != NULL);
    saved_len = fread(saved, 1, sizeof(saved) - 1, fp);
    fclose(fp);
    saved[saved_len] = '\0';
    assert(strcmp(saved, "prompted") == 0);
    editor_free(&editor);

    editor_init(&editor);
    editor_insert_row(&editor, 0, "cancel", 6);
    editor.dirty = 1;
    {
        int pipe_fds[2];
        int saved_stdin;
        int saved_stdout;
        int dev_null;

        assert(pipe(pipe_fds) == 0);
        assert(write(pipe_fds[1], "\x1b", 1) == 1);
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

        assert(fileio_save(&editor) == -1);

        assert(dup2(saved_stdout, STDOUT_FILENO) != -1);
        close(saved_stdout);
        assert(dup2(saved_stdin, STDIN_FILENO) != -1);
        close(saved_stdin);
    }
    assert(editor.filename == NULL);
    assert(editor.dirty == 1);
    assert(strcmp(editor.status_message, "Save cancelled") == 0);
    editor_free(&editor);

    unlink(path);
    unlink(save_path);
    unlink(prompt_save_path);
    return 0;
}
