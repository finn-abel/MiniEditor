#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "editor.h"

static void test_insert_and_delete_rows(void)
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

    /* Out-of-range insertion points are ignored. */
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

    /* Out-of-range deletes are ignored. */
    editor_delete_row(&editor, -1);
    editor_delete_row(&editor, 4);
    assert(editor.row_count == 2);
    assert(editor.dirty == 4);

    editor_free(&editor);
}

static void test_insert_char_clamps_cursor(void)
{
    Editor editor;

    editor_init(&editor);

    /* An out-of-range cursor is clamped before inserting into a new row. */
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

    editor_free(&editor);
}

static void test_newline_splits_row(void)
{
    Editor editor;

    editor_init(&editor);
    editor_insert_char(&editor, 'h');
    editor_insert_char(&editor, '!');
    editor_insert_char(&editor, 'i');

    editor.cursor_x = 2;
    editor_insert_newline(&editor);
    assert(editor.row_count == 2);
    assert(strcmp(editor.rows[0].chars, "h!") == 0);
    assert(strcmp(editor.rows[1].chars, "i") == 0);
    assert(editor.cursor_y == 1);
    assert(editor.cursor_x == 0);

    editor_free(&editor);
}

static void test_newline_appends_when_cursor_below_buffer(void)
{
    Editor editor;

    editor_init(&editor);
    editor_insert_char(&editor, 'a');
    editor_insert_char(&editor, 'b');

    editor.cursor_y = 99;
    editor.cursor_x = 99;
    editor_insert_newline(&editor);
    assert(editor.row_count == 2);
    assert(editor.cursor_y == 1);
    assert(editor.cursor_x == 0);
    assert(strcmp(editor.rows[1].chars, "") == 0);

    editor_free(&editor);
}

static void test_delete_char_within_row(void)
{
    Editor editor;

    editor_init(&editor);
    editor_insert_row(&editor, 0, "h!oi", 4);
    editor.cursor_y = 0;
    editor.cursor_x = 3;
    editor_delete_char(&editor);
    assert(strcmp(editor.rows[0].chars, "h!i") == 0);
    assert(editor.cursor_x == 2);

    editor_free(&editor);
}

static void test_delete_char_joins_rows(void)
{
    Editor editor;

    editor_init(&editor);
    editor_insert_row(&editor, 0, "ab", 2);
    editor_insert_row(&editor, 1, "cd", 2);

    editor.cursor_y = 1;
    editor.cursor_x = 0;
    editor_delete_char(&editor);
    assert(editor.row_count == 1);
    assert(editor.cursor_y == 0);
    assert(editor.cursor_x == 2);
    assert(strcmp(editor.rows[0].chars, "abcd") == 0);

    editor_free(&editor);
}

static void test_delete_char_noop_at_origin(void)
{
    Editor editor;
    int dirty_before;

    editor_init(&editor);
    editor_insert_row(&editor, 0, "abc", 3);
    editor.cursor_y = 0;
    editor.cursor_x = 0;
    dirty_before = editor.dirty;

    editor_delete_char(&editor);
    assert(editor.dirty == dirty_before);
    assert(strcmp(editor.rows[0].chars, "abc") == 0);

    editor_free(&editor);
}

static void test_newline_at_edges_and_serialize(void)
{
    Editor editor;
    char *serialized;
    int serialized_len;

    editor_init(&editor);
    editor_insert_char(&editor, 'h');
    editor_insert_char(&editor, '!');
    editor_insert_char(&editor, 'i');

    /* Newline at column 0 opens a blank line above the text. */
    editor.cursor_x = 0;
    editor_insert_newline(&editor);
    assert(editor.row_count == 2);
    assert(strcmp(editor.rows[0].chars, "") == 0);
    assert(strcmp(editor.rows[1].chars, "h!i") == 0);

    /* Newline at end of a row appends a trailing blank line. */
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
}

int main(void)
{
    test_insert_and_delete_rows();
    test_insert_char_clamps_cursor();
    test_newline_splits_row();
    test_newline_appends_when_cursor_below_buffer();
    test_delete_char_within_row();
    test_delete_char_joins_rows();
    test_delete_char_noop_at_origin();
    test_newline_at_edges_and_serialize();
    return 0;
}
