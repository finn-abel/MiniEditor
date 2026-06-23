#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "editor.h"

int main(void)
{
    Editor editor;
    char *serialized;
    int serialized_len;

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

    editor_init(&editor);
    editor.cursor_y = 99;
    editor.cursor_x = 99;
    editor_insert_char(&editor, 'h');
    editor_insert_char(&editor, 'i');
    assert(editor.row_count == 1);
    assert(editor.cursor_x == 2);
    assert(editor.cursor_y == 0);
    assert(editor.dirty == 2);
    assert(strcmp(editor.rows[0].chars, "hi") == 0);

    editor.cursor_x = 1;
    editor_insert_char(&editor, '!');
    assert(strcmp(editor.rows[0].chars, "h!i") == 0);
    assert(editor.cursor_x == 2);
    assert(editor.dirty == 3);

    editor_insert_newline(&editor);
    assert(editor.row_count == 2);
    assert(editor.cursor_x == 0);
    assert(editor.cursor_y == 1);
    assert(strcmp(editor.rows[0].chars, "h!") == 0);
    assert(strcmp(editor.rows[1].chars, "i") == 0);

    editor_insert_char(&editor, 'w');
    editor_insert_char(&editor, 'o');
    assert(strcmp(editor.rows[1].chars, "woi") == 0);

    editor.cursor_y = 99;
    editor.cursor_x = 99;
    editor_insert_newline(&editor);
    assert(editor.row_count == 3);
    assert(editor.cursor_y == 2);
    assert(editor.cursor_x == 0);
    assert(strcmp(editor.rows[2].chars, "") == 0);

    editor_delete_char(&editor);
    assert(editor.row_count == 2);
    assert(editor.cursor_y == 1);
    assert(editor.cursor_x == 3);
    assert(strcmp(editor.rows[1].chars, "woi") == 0);

    editor.cursor_x = 1;
    editor.cursor_y = 1;
    editor_delete_char(&editor);
    assert(strcmp(editor.rows[1].chars, "oi") == 0);
    assert(editor.cursor_x == 0);

    editor_delete_char(&editor);
    assert(editor.row_count == 1);
    assert(editor.cursor_y == 0);
    assert(editor.cursor_x == 2);
    assert(strcmp(editor.rows[0].chars, "h!oi") == 0);

    editor.cursor_x = 0;
    editor.cursor_y = 0;
    {
        int dirty_before = editor.dirty;

        editor_delete_char(&editor);
        assert(editor.dirty == dirty_before);
        assert(strcmp(editor.rows[0].chars, "h!oi") == 0);
    }

    editor.cursor_x = 2;
    editor.cursor_y = 0;
    editor.cursor_x++;
    editor_delete_char(&editor);
    assert(strcmp(editor.rows[0].chars, "h!i") == 0);

    editor.cursor_x = 0;
    editor_insert_newline(&editor);
    assert(editor.row_count == 2);
    assert(strcmp(editor.rows[0].chars, "") == 0);
    assert(strcmp(editor.rows[1].chars, "h!i") == 0);

    editor.cursor_y = 1;
    editor.cursor_x = editor.rows[1].size;
    editor_insert_newline(&editor);
    assert(editor.row_count == 3);
    assert(strcmp(editor.rows[2].chars, "") == 0);

    serialized = editor_rows_to_string(&editor, &serialized_len);
    assert(serialized != NULL);
    assert(serialized_len == 5);
    assert(strcmp(serialized, "\nh!i\n") == 0);
    free(serialized);

    editor_free(&editor);
    return 0;
}
