#include <assert.h>
#include <string.h>

#include "row.h"

static void test_tab_rendering_and_positions(void)
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
    /* Out-of-range raw position clamps to the row size. */
    assert(row_cursor_x_to_render_x(&row, 99) == 9);
    row_free(&row);
}

static void test_render_x_to_cursor_x(void)
{
    EditorRow row;

    row_init(&row, 0, "a\tb", 3);
    assert(row_render_x_to_cursor_x(&row, 0) == 0);
    assert(row_render_x_to_cursor_x(&row, -2) == 0);
    assert(row_render_x_to_cursor_x(&row, 8) == 2);
    /* A render column past the end maps back to the final raw position. */
    assert(row_render_x_to_cursor_x(&row, 50) == 3);
    row_free(&row);
}

static void test_insert_and_append(void)
{
    EditorRow row;

    row_init(&row, 0, "helo", 4);
    row_insert_char(&row, 2, 'l');
    assert(strcmp(row.chars, "hello") == 0);
    assert(strcmp(row.render, "hello") == 0);

    /* Negative index clamps to the start, past-end clamps to the end. */
    row_insert_char(&row, -10, '>');
    assert(strcmp(row.chars, ">hello") == 0);
    row_insert_char(&row, 999, '<');
    assert(strcmp(row.chars, ">hello<") == 0);

    row_append_string(&row, "!?", 2);
    assert(strcmp(row.chars, ">hello<!?") == 0);
    assert(row.size == 9);
    row_free(&row);
}

static void test_delete_char(void)
{
    EditorRow row;

    row_init(&row, 0, ">hello!", 7);
    row_delete_char(&row, 0);
    assert(strcmp(row.chars, "hello!") == 0);

    row_delete_char(&row, row.size - 1);
    assert(strcmp(row.chars, "hello") == 0);

    /* Out-of-bounds deletes are no-ops. */
    row_delete_char(&row, -1);
    row_delete_char(&row, row.size);
    row_delete_char(&row, 999);
    assert(strcmp(row.chars, "hello") == 0);
    assert(row.size == 5);
    row_free(&row);
}

int main(void)
{
    test_tab_rendering_and_positions();
    test_render_x_to_cursor_x();
    test_insert_and_append();
    test_delete_char();
    return 0;
}
