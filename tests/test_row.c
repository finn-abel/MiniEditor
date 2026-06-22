#include <assert.h>
#include <string.h>

#include "row.h"

int main(void)
{
    EditorRow row;

    row_init(&row, 0, "a\tb", 3);
    assert(row.index == 0);
    assert(row.size == 3);
    assert(row.render_size == 9);
    assert(strcmp(row.render, "a       b") == 0);
    assert(row_cursor_x_to_render_x(&row, 0) == 0);
    assert(row_cursor_x_to_render_x(&row, -3) == 0);
    assert(row_cursor_x_to_render_x(&row, 1) == 1);
    assert(row_cursor_x_to_render_x(&row, 2) == 8);
    assert(row_cursor_x_to_render_x(&row, 3) == 9);
    assert(row_render_x_to_cursor_x(&row, 0) == 0);
    assert(row_render_x_to_cursor_x(&row, -2) == 0);
    assert(row_render_x_to_cursor_x(&row, 8) == 2);
    row_free(&row);

    row_init(&row, 0, "helo", 4);
    row_insert_char(&row, 2, 'l');
    assert(strcmp(row.chars, "hello") == 0);
    assert(strcmp(row.render, "hello") == 0);

    row_insert_char(&row, -10, '>');
    assert(strcmp(row.chars, ">hello") == 0);

    row_append_string(&row, "!", 1);
    assert(strcmp(row.chars, ">hello!") == 0);

    row_delete_char(&row, 0);
    assert(strcmp(row.chars, "hello!") == 0);

    row_delete_char(&row, row.size - 1);
    assert(strcmp(row.chars, "hello") == 0);
    row_free(&row);

    return 0;
}
