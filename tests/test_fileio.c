#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "editor.h"
#include "fileio.h"

int main(void)
{
    Editor editor;
    const char *path = "/tmp/minieditor_fileio_test.txt";
    FILE *fp;

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
    unlink(path);
    return 0;
}
