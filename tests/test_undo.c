#include <assert.h>
#include <string.h>

#include "buffer.h"
#include "editor.h"
#include "undo.h"

int main(void)
{
    Editor editor;

    /* --- undo insert char --- */
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
    /* empty buffer: row created with char was also auto-created, so undoing
       the first char should remove the row entirely */
    assert(editor.row_count == 0);
    assert(!undo_can_undo(&editor.undo_stack));

    editor_free(&editor);

    /* --- undo delete char --- */
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

    /* --- undo insert newline (mid-row split) --- */
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

    /* --- undo insert newline (at end of row) --- */
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

    /* --- undo insert newline (at start of row) --- */
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

    /* --- undo join row (backspace at col 0) --- */
    editor_init(&editor);

    editor_insert_char(&editor, 'a');
    editor_insert_char(&editor, 'b');
    editor_insert_newline(&editor);
    editor_insert_char(&editor, 'c');
    editor_insert_char(&editor, 'd');
    assert(editor.row_count == 2);
    assert(strcmp(editor.rows[0].chars, "ab") == 0);
    assert(strcmp(editor.rows[1].chars, "cd") == 0);

    /* backspace at col 0 of row 1 joins rows */
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

    /* --- redo after undo --- */
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

    /* --- redo stack cleared on new edit after undo --- */
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

    /* --- dirty flag: undo back to saved state --- */
    editor_init(&editor);

    editor_insert_char(&editor, 'a');
    editor_insert_char(&editor, 'b');
    assert(editor.dirty > 0);

    /* simulate a save */
    editor.dirty = 0;
    undo_mark_saved(&editor.undo_stack);

    editor_insert_char(&editor, 'c');
    assert(editor.dirty > 0);

    undo_undo(&editor);
    assert(editor.dirty == 0);

    undo_undo(&editor);
    assert(editor.dirty > 0);

    editor_free(&editor);

    /* --- undo_can_undo / undo_can_redo on empty stack --- */
    editor_init(&editor);
    assert(!undo_can_undo(&editor.undo_stack));
    assert(!undo_can_redo(&editor.undo_stack));
    undo_undo(&editor);  /* no-op, no crash */
    undo_redo(&editor);  /* no-op, no crash */
    editor_free(&editor);

    /* --- stack cap: push beyond UNDO_MAX, oldest evicted --- */
    editor_init(&editor);

    {
        int i;
        /* Insert UNDO_MAX + 1 characters. The first character's undo entry
           will be evicted to make room. */
        for (i = 0; i < UNDO_MAX + 1; i++) {
            editor_insert_char(&editor, 'x');
        }
        assert(editor.undo_stack.count == UNDO_MAX);
        assert(editor.undo_stack.position == UNDO_MAX);

        /* Undo UNDO_MAX times should leave one char that can't be undone. */
        for (i = 0; i < UNDO_MAX; i++) {
            undo_undo(&editor);
        }
        assert(!undo_can_undo(&editor.undo_stack));
        /* One character remains because its entry was evicted. */
        assert(editor.row_count == 1);
        assert(editor.rows[0].size == 1);
    }

    editor_free(&editor);

    return 0;
}
