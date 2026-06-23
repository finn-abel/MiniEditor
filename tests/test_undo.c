#include <assert.h>
#include <string.h>

#include "buffer.h"
#include "editor.h"
#include "undo.h"

static void test_undo_insert_char(void)
{
    Editor editor;

    editor_init(&editor);

    editor_insert_char(&editor, 'a');
    editor_insert_char(&editor, 'b');
    editor_insert_char(&editor, 'c');
    assert(editor.row_count == 1);
    assert(strcmp(editor.rows[0].chars, "abc") == 0);
    assert(editor.cursor_x == 3);

    undo_undo(&editor);
    assert(strcmp(editor.rows[0].chars, "ab") == 0);
    assert(editor.cursor_x == 2);

    undo_undo(&editor);
    assert(strcmp(editor.rows[0].chars, "a") == 0);

    undo_undo(&editor);
    /* The first char also auto-created the row, so undoing it removes the
       row entirely. */
    assert(editor.row_count == 0);
    assert(!undo_can_undo(&editor.undo_stack));

    editor_free(&editor);
}

static void test_undo_delete_char(void)
{
    Editor editor;

    editor_init(&editor);

    editor_insert_char(&editor, 'x');
    editor_insert_char(&editor, 'y');
    editor_insert_char(&editor, 'z');
    assert(strcmp(editor.rows[0].chars, "xyz") == 0);

    editor_delete_char(&editor);
    assert(strcmp(editor.rows[0].chars, "xy") == 0);

    undo_undo(&editor);
    assert(strcmp(editor.rows[0].chars, "xyz") == 0);
    assert(editor.cursor_x == 3);

    editor_free(&editor);
}

static void test_undo_newline_mid_row(void)
{
    Editor editor;

    editor_init(&editor);

    editor_insert_char(&editor, 'h');
    editor_insert_char(&editor, 'i');
    editor.cursor_x = 1;
    editor_insert_newline(&editor);
    assert(editor.row_count == 2);
    assert(strcmp(editor.rows[0].chars, "h") == 0);
    assert(strcmp(editor.rows[1].chars, "i") == 0);
    assert(editor.cursor_y == 1);
    assert(editor.cursor_x == 0);

    undo_undo(&editor);
    assert(editor.row_count == 1);
    assert(strcmp(editor.rows[0].chars, "hi") == 0);
    assert(editor.cursor_y == 0);
    assert(editor.cursor_x == 1);

    editor_free(&editor);
}

static void test_undo_newline_end_of_row(void)
{
    Editor editor;

    editor_init(&editor);

    editor_insert_char(&editor, 'a');
    editor_insert_char(&editor, 'b');
    editor_insert_newline(&editor);
    assert(editor.row_count == 2);
    assert(strcmp(editor.rows[0].chars, "ab") == 0);
    assert(strcmp(editor.rows[1].chars, "") == 0);

    undo_undo(&editor);
    assert(editor.row_count == 1);
    assert(strcmp(editor.rows[0].chars, "ab") == 0);

    editor_free(&editor);
}

static void test_undo_newline_start_of_row(void)
{
    Editor editor;

    editor_init(&editor);

    editor_insert_char(&editor, 'a');
    editor.cursor_x = 0;
    editor_insert_newline(&editor);
    assert(editor.row_count == 2);
    assert(strcmp(editor.rows[0].chars, "") == 0);
    assert(strcmp(editor.rows[1].chars, "a") == 0);

    undo_undo(&editor);
    assert(editor.row_count == 1);
    assert(strcmp(editor.rows[0].chars, "a") == 0);
    assert(editor.cursor_y == 0);
    assert(editor.cursor_x == 0);

    editor_free(&editor);
}

static void test_undo_join_row(void)
{
    Editor editor;

    editor_init(&editor);

    editor_insert_char(&editor, 'a');
    editor_insert_char(&editor, 'b');
    editor_insert_newline(&editor);
    editor_insert_char(&editor, 'c');
    editor_insert_char(&editor, 'd');
    assert(editor.row_count == 2);
    assert(strcmp(editor.rows[0].chars, "ab") == 0);
    assert(strcmp(editor.rows[1].chars, "cd") == 0);

    /* Backspace at col 0 of row 1 joins the rows. */
    editor.cursor_y = 1;
    editor.cursor_x = 0;
    editor_delete_char(&editor);
    assert(editor.row_count == 1);
    assert(strcmp(editor.rows[0].chars, "abcd") == 0);
    assert(editor.cursor_y == 0);
    assert(editor.cursor_x == 2);

    undo_undo(&editor);
    assert(editor.row_count == 2);
    assert(strcmp(editor.rows[0].chars, "ab") == 0);
    assert(strcmp(editor.rows[1].chars, "cd") == 0);
    assert(editor.cursor_y == 1);
    assert(editor.cursor_x == 0);

    editor_free(&editor);
}

static void test_redo_insert_char(void)
{
    Editor editor;

    editor_init(&editor);

    editor_insert_char(&editor, 'p');
    editor_insert_char(&editor, 'q');

    undo_undo(&editor);
    assert(strcmp(editor.rows[0].chars, "p") == 0);
    assert(undo_can_redo(&editor.undo_stack));

    undo_redo(&editor);
    assert(strcmp(editor.rows[0].chars, "pq") == 0);
    assert(!undo_can_redo(&editor.undo_stack));

    editor_free(&editor);
}

static void test_redo_delete_char(void)
{
    Editor editor;

    editor_init(&editor);

    editor_insert_char(&editor, 'x');
    editor_insert_char(&editor, 'y');
    editor_insert_char(&editor, 'z');
    editor_delete_char(&editor);
    assert(strcmp(editor.rows[0].chars, "xy") == 0);

    undo_undo(&editor);
    assert(strcmp(editor.rows[0].chars, "xyz") == 0);

    undo_redo(&editor);
    assert(strcmp(editor.rows[0].chars, "xy") == 0);

    editor_free(&editor);
}

static void test_redo_insert_newline(void)
{
    Editor editor;

    editor_init(&editor);

    editor_insert_char(&editor, 'h');
    editor_insert_char(&editor, 'i');
    editor.cursor_x = 1;
    editor_insert_newline(&editor);
    assert(editor.row_count == 2);

    undo_undo(&editor);
    assert(editor.row_count == 1);
    assert(strcmp(editor.rows[0].chars, "hi") == 0);

    undo_redo(&editor);
    assert(editor.row_count == 2);
    assert(strcmp(editor.rows[0].chars, "h") == 0);
    assert(strcmp(editor.rows[1].chars, "i") == 0);

    editor_free(&editor);
}

static void test_redo_join_row(void)
{
    Editor editor;

    editor_init(&editor);

    editor_insert_char(&editor, 'a');
    editor_insert_char(&editor, 'b');
    editor_insert_newline(&editor);
    editor_insert_char(&editor, 'c');
    editor_insert_char(&editor, 'd');

    editor.cursor_y = 1;
    editor.cursor_x = 0;
    editor_delete_char(&editor);
    assert(editor.row_count == 1);
    assert(strcmp(editor.rows[0].chars, "abcd") == 0);

    undo_undo(&editor);
    assert(editor.row_count == 2);
    assert(strcmp(editor.rows[1].chars, "cd") == 0);

    undo_redo(&editor);
    assert(editor.row_count == 1);
    assert(strcmp(editor.rows[0].chars, "abcd") == 0);

    editor_free(&editor);
}

static void test_redo_cleared_on_new_edit(void)
{
    Editor editor;

    editor_init(&editor);

    editor_insert_char(&editor, '1');
    editor_insert_char(&editor, '2');
    editor_insert_char(&editor, '3');

    undo_undo(&editor);
    undo_undo(&editor);
    assert(strcmp(editor.rows[0].chars, "1") == 0);
    assert(undo_can_redo(&editor.undo_stack));

    editor_insert_char(&editor, 'x');
    assert(strcmp(editor.rows[0].chars, "1x") == 0);
    assert(!undo_can_redo(&editor.undo_stack));

    editor_free(&editor);
}

static void test_dirty_tracks_saved_position(void)
{
    Editor editor;

    editor_init(&editor);

    editor_insert_char(&editor, 'a');
    editor_insert_char(&editor, 'b');
    assert(editor.dirty > 0);

    /* Simulate a save at the current position. */
    editor.dirty = 0;
    undo_mark_saved(&editor.undo_stack);

    editor_insert_char(&editor, 'c');
    assert(editor.dirty > 0);

    undo_undo(&editor);
    assert(editor.dirty == 0);

    undo_undo(&editor);
    assert(editor.dirty > 0);

    editor_free(&editor);
}

static void test_undo_redo_empty_stack_noop(void)
{
    Editor editor;

    editor_init(&editor);
    assert(!undo_can_undo(&editor.undo_stack));
    assert(!undo_can_redo(&editor.undo_stack));
    undo_undo(&editor); /* no-op, no crash */
    undo_redo(&editor); /* no-op, no crash */
    editor_free(&editor);
}

static void test_stack_cap_evicts_oldest(void)
{
    Editor editor;
    int i;

    editor_init(&editor);

    /* Insert UNDO_MAX + 1 characters. The first character's undo entry is
       evicted to make room. */
    for (i = 0; i < UNDO_MAX + 1; i++) {
        editor_insert_char(&editor, 'x');
    }
    assert(editor.undo_stack.count == UNDO_MAX);
    assert(editor.undo_stack.position == UNDO_MAX);

    for (i = 0; i < UNDO_MAX; i++) {
        undo_undo(&editor);
    }
    assert(!undo_can_undo(&editor.undo_stack));
    /* One character remains because its entry was evicted. */
    assert(editor.row_count == 1);
    assert(editor.rows[0].size == 1);

    editor_free(&editor);
}

int main(void)
{
    test_undo_insert_char();
    test_undo_delete_char();
    test_undo_newline_mid_row();
    test_undo_newline_end_of_row();
    test_undo_newline_start_of_row();
    test_undo_join_row();
    test_redo_insert_char();
    test_redo_delete_char();
    test_redo_insert_newline();
    test_redo_join_row();
    test_redo_cleared_on_new_edit();
    test_dirty_tracks_saved_position();
    test_undo_redo_empty_stack_noop();
    test_stack_cap_evicts_oldest();
    return 0;
}
