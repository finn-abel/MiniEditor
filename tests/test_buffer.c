#include <assert.h>
#include <string.h>

#include "buffer.h"
#include "editor.h"

int main(void)
{
    Editor editor;

    editor_init(&editor);

    editor_insert_row(&editor, 0, "second", 6);
    editor_insert_row(&editor, 0, "first", 5);
    editor_insert_row(&editor, 2, "third", 5);

    assert(editor.row_count == 3);
    assert(editor.dirty == 3);
    assert(editor.rows[0].index == 0);
    assert(editor.rows[1].index == 1);
    assert(editor.rows[2].index == 2);
    assert(strcmp(editor.rows[0].chars, "first") == 0);
    assert(strcmp(editor.rows[1].chars, "second") == 0);
    assert(strcmp(editor.rows[2].chars, "third") == 0);

    editor_insert_row(&editor, -1, "bad", 3);
    editor_insert_row(&editor, 5, "bad", 3);
    assert(editor.row_count == 3);
    assert(editor.dirty == 3);

    editor_delete_row(&editor, 1);
    assert(editor.row_count == 2);
    assert(editor.dirty == 4);
    assert(editor.rows[0].index == 0);
    assert(editor.rows[1].index == 1);
    assert(strcmp(editor.rows[0].chars, "first") == 0);
    assert(strcmp(editor.rows[1].chars, "third") == 0);

    editor_delete_row(&editor, -1);
    editor_delete_row(&editor, 4);
    assert(editor.row_count == 2);
    assert(editor.dirty == 4);

    editor_free(&editor);
    return 0;
}
