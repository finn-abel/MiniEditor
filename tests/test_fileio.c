#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "editor.h"
#include "fileio.h"

// Replace editor->filename with a heap copy of path (editor_free owns it).
static void set_filename(Editor *editor, const char *path)
{
    free(editor->filename);
    editor->filename = malloc(strlen(path) + 1);
    assert(editor->filename != NULL);
    strcpy(editor->filename, path);
}

// Call fileio_save while feeding prompt keystrokes on stdin and discarding the
// redraw on stdout. Used for the save-as prompt and cancel paths.
static int save_with_stdin(Editor *editor, const char *input, int len)
{
    int pipe_fds[2];
    int saved_stdin;
    int saved_stdout;
    int dev_null;
    int result;

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

    result = fileio_save(editor);

    assert(dup2(saved_stdout, STDOUT_FILENO) != -1);
    close(saved_stdout);
    assert(dup2(saved_stdin, STDIN_FILENO) != -1);
    close(saved_stdin);
    return result;
}

static void test_open_existing_file(void)
{
    Editor editor;
    const char *path = "/tmp/minieditor_fileio_test.txt";
    FILE *fp = fopen(path, "w");

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
    unlink(path);
}

static void test_open_missing_file(void)
{
    Editor editor;
    const char *missing_path = "/tmp/minieditor_missing_file.txt";

    unlink(missing_path);
    editor_init(&editor);
    fileio_open(&editor, missing_path);
    assert(editor.filename != NULL);
    assert(strcmp(editor.filename, missing_path) == 0);
    assert(editor.row_count == 0);
    assert(editor.dirty == 0);
    assert(strstr(editor.status_message, "New file") != NULL);
    editor_free(&editor);
}

static void test_open_directory_reports_error(void)
{
    Editor editor;

    editor_init(&editor);
    fileio_open(&editor, "/tmp");
    assert(editor.row_count == 0);
    assert(editor.dirty == 0);
    assert(strstr(editor.status_message, "Is a directory") != NULL);
    editor_free(&editor);
}

static void test_save_writes_file(void)
{
    Editor editor;
    const char *save_path = "/tmp/minieditor_save_test.txt";
    FILE *fp;
    char saved[64];
    size_t saved_len;

    editor_init(&editor);
    set_filename(&editor, save_path);
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
    unlink(save_path);
}

static void test_save_failure_on_directory_path(void)
{
    Editor editor;

    editor_init(&editor);
    set_filename(&editor, "/tmp/");
    editor_insert_row(&editor, 0, "cannot save here", 16);
    editor.dirty = 1;

    assert(fileio_save(&editor) == -1);
    assert(editor.dirty == 1);
    assert(strstr(editor.status_message, "Save failed") != NULL);

    editor_free(&editor);
}

static void test_save_prompts_for_filename(void)
{
    Editor editor;
    const char *prompt_save_path = "/tmp/minieditor_prompt_save_test.txt";
    char input[128];
    int input_len;
    FILE *fp;
    char saved[64];
    size_t saved_len;

    editor_init(&editor);
    editor_insert_row(&editor, 0, "prompted", 8);
    editor.dirty = 1;

    input_len = snprintf(input, sizeof(input), "%s\r", prompt_save_path);
    assert(save_with_stdin(&editor, input, input_len) == 0);
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
    unlink(prompt_save_path);
}

static void test_save_canceled_at_prompt(void)
{
    Editor editor;

    editor_init(&editor);
    editor_insert_row(&editor, 0, "cancel", 6);
    editor.dirty = 1;

    assert(save_with_stdin(&editor, "\x1b", 1) == -1);
    assert(editor.filename == NULL);
    assert(editor.dirty == 1);
    assert(strcmp(editor.status_message, "Save canceled") == 0);

    editor_free(&editor);
}

int main(void)
{
    test_open_existing_file();
    test_open_missing_file();
    test_open_directory_reports_error();
    test_save_writes_file();
    test_save_failure_on_directory_path();
    test_save_prompts_for_filename();
    test_save_canceled_at_prompt();
    return 0;
}
