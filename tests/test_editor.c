#include <assert.h>

#include "buffer.h"
#include "editor.h"

int main(void)
{
    Editor editor;

    editor_init(&editor);
    assert(editor.initialized == 1);
    assert(editor.screen_needs_clear == 1);
    assert(editor.filename == NULL);
    assert(editor.status_message[0] == '\0');
    assert(editor.status_message_time == 0);
    assert(editor.render_x == 0);
    assert(editor.row_offset == 0);
    assert(editor.col_offset == 0);

    editor.screen_rows = 3;
    editor.screen_cols = 10;
    editor.line_number_width = 1;
    editor_insert_row(&editor, 0, "01234567890123456789", 20);
    editor_insert_row(&editor, 1, "next", 4);
    editor.cursor_y = 1;
    editor.cursor_x = 4;
    editor_scroll(&editor);
    assert(editor.render_x == 4);
    assert(editor.row_offset == 0);

    editor.cursor_y = 4;
    editor_scroll(&editor);
    assert(editor.row_offset == 2);

    editor.cursor_y = 0;
    editor.cursor_x = 19;
    editor_scroll(&editor);
    assert(editor.row_offset == 0);
    assert(editor.col_offset > 0);

    editor_free(&editor);
    assert(editor.initialized == 0);
    assert(editor.screen_needs_clear == 0);

    return 0;
}
