#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "editor.h"
#include "syntax.h"

// Build an editor with a ".c" filename and a small C program so syntax
// highlighting is active. Caller frees with editor_free.
static void setup_c_editor(Editor *editor)
{
    editor_init(editor);
    editor->filename = malloc(strlen("highlight.c") + 1);
    assert(editor->filename != NULL);
    strcpy(editor->filename, "highlight.c");

    editor_insert_row(editor, 0, "#include <stdio.h>", 18);
    editor_insert_row(editor, 1, "int main(void) {", 16);
    editor_insert_row(editor, 2, "    // greeting", 15);
    editor_insert_row(editor, 3, "    printf(\"hello %d\\n\", 123);", 30);
    editor_insert_row(editor, 4, "    return 0;", 13);
    editor_insert_row(editor, 5, "/* open", 7);
    editor_insert_row(editor, 6, "still comment */ int x;", 22);
    syntax_select(editor);
}

static void test_filetype_detection(void)
{
    Editor editor;

    setup_c_editor(&editor);
    assert(editor.syntax != NULL);
    assert(strcmp(editor.syntax->filetype, "C") == 0);
    editor_free(&editor);
}

static void test_unknown_filetype_has_no_syntax(void)
{
    Editor editor;

    editor_init(&editor);
    editor.filename = malloc(strlen("notes.unknownext") + 1);
    assert(editor.filename != NULL);
    strcpy(editor.filename, "notes.unknownext");
    editor_insert_row(&editor, 0, "int x;", 6);
    syntax_select(&editor);
    assert(editor.syntax == NULL);
    editor_free(&editor);
}

static void test_highlight_classes(void)
{
    Editor editor;
    int index;

    setup_c_editor(&editor);

    assert(editor.rows[0].highlight[0] == HL_PREPROCESSOR);
    assert(editor.rows[1].highlight[0] == HL_KEYWORD2);
    assert(editor.rows[1].highlight[9] == HL_KEYWORD2);
    assert(editor.rows[2].highlight[4] == HL_COMMENT);

    for (index = 11; index < 23; index++) {
        assert(editor.rows[3].highlight[index] == HL_STRING);
    }
    assert(editor.rows[3].highlight[26] == HL_NUMBER);
    assert(editor.rows[4].highlight[4] == HL_KEYWORD1);

    editor_free(&editor);
}

static void test_multiline_comment_carries_forward(void)
{
    Editor editor;

    setup_c_editor(&editor);
    assert(editor.rows[5].highlight_open_comment == 1);
    assert(editor.rows[6].highlight[0] == HL_MLCOMMENT);
    editor_free(&editor);
}

static void test_syntax_color_codes(void)
{
    assert(syntax_color(HL_NORMAL) == 39);
    assert(syntax_color(HL_COMMENT) == 90);
    assert(syntax_color(HL_MLCOMMENT) == 90);
    assert(syntax_color(HL_KEYWORD1) == 33);
    assert(syntax_color(HL_KEYWORD2) == 32);
    assert(syntax_color(HL_STRING) == 35);
    assert(syntax_color(HL_NUMBER) == 31);
    assert(syntax_color(HL_MATCH) == 34);
    assert(syntax_color(HL_PREPROCESSOR) == 36);
    /* Any unknown highlight falls back to the default foreground color. */
    assert(syntax_color(-999) == 39);
}

int main(void)
{
    test_filetype_detection();
    test_unknown_filetype_has_no_syntax();
    test_highlight_classes();
    test_multiline_comment_carries_forward();
    test_syntax_color_codes();
    return 0;
}
