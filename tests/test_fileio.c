#include <assert.h>
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

    unlink(path);
    unlink(save_path);
    return 0;
}
