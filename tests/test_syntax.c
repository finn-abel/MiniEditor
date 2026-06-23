#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "editor.h"
#include "syntax.h"

int main(void)
{
    Editor editor;
    int index;

    editor_init(&editor);
    editor.filename = malloc(strlen("highlight.c") + 1);
    assert(editor.filename != NULL);
    strcpy(editor.filename, "highlight.c");

    editor_insert_row(&editor, 0, "#include <stdio.h>", 18);
    editor_insert_row(&editor, 1, "int main(void) {", 16);
    editor_insert_row(&editor, 2, "    // greeting", 15);
    editor_insert_row(&editor, 3, "    printf(\"hello %d\\n\", 123);", 30);
    editor_insert_row(&editor, 4, "    return 0;", 13);
    editor_insert_row(&editor, 5, "/* open", 7);
    editor_insert_row(&editor, 6, "still comment */ int x;", 22);
    syntax_select(&editor);

    assert(editor.syntax != NULL);
    assert(strcmp(editor.syntax->filetype, "C") == 0);

    assert(editor.rows[0].highlight[0] == HL_PREPROCESSOR);
    assert(editor.rows[1].highlight[0] == HL_KEYWORD2);
    assert(editor.rows[1].highlight[9] == HL_KEYWORD2);
    assert(editor.rows[2].highlight[4] == HL_COMMENT);

    for (index = 11; index < 23; index++) {
        assert(editor.rows[3].highlight[index] == HL_STRING);
    }
    assert(editor.rows[3].highlight[26] == HL_NUMBER);
    assert(editor.rows[4].highlight[4] == HL_KEYWORD1);
    assert(editor.rows[5].highlight_open_comment == 1);
    assert(editor.rows[6].highlight[0] == HL_MLCOMMENT);

    assert(syntax_color(HL_PREPROCESSOR) == 36);
    assert(syntax_color(HL_MATCH) == 34);

    editor_free(&editor);
    return 0;
}
