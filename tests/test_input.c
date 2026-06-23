#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "editor.h"
#include "input.h"

// Feed raw key bytes to input_process_keypress by redirecting stdin from a
// pipe, then restore the real stdin so other cases are unaffected.
static void process_key_bytes(Editor *editor, const char *keys, int len)
{
    int pipe_fds[2];
    int saved_stdin;

    assert(pipe(pipe_fds) == 0);
    assert(write(pipe_fds[1], keys, (size_t) len) == len);
    close(pipe_fds[1]);

    saved_stdin = dup(STDIN_FILENO);
    assert(saved_stdin != -1);
    assert(dup2(pipe_fds[0], STDIN_FILENO) != -1);
    close(pipe_fds[0]);

    input_process_keypress(editor);

    assert(dup2(saved_stdin, STDIN_FILENO) != -1);
    close(saved_stdin);
}

static void test_arrow_keys_navigate(void)
{
    Editor editor;

    editor_init(&editor);
    editor_insert_row(&editor, 0, "hello", 5);
    editor_insert_row(&editor, 1, "world", 5);
    editor.cursor_x = 0;
    editor.cursor_y = 0;

    process_key_bytes(&editor, "\x1b[C", 3); /* right */
    assert(editor.cursor_x == 1 && editor.cursor_y == 0);
    process_key_bytes(&editor, "\x1b[B", 3); /* down */
    assert(editor.cursor_y == 1);
    process_key_bytes(&editor, "\x1b[D", 3); /* left */
    assert(editor.cursor_x == 0);
    process_key_bytes(&editor, "\x1b[A", 3); /* up */
    assert(editor.cursor_y == 0);

    editor_free(&editor);
}

static void test_arrow_wraps_between_rows(void)
{
    Editor editor;

    editor_init(&editor);
    editor_insert_row(&editor, 0, "ab", 2);
    editor_insert_row(&editor, 1, "cd", 2);
    editor.cursor_y = 0;
    editor.cursor_x = 2;

    /* Right at end of a row wraps to the start of the next. */
    process_key_bytes(&editor, "\x1b[C", 3);
    assert(editor.cursor_y == 1 && editor.cursor_x == 0);
    /* Left at column 0 wraps to the end of the previous row. */
    process_key_bytes(&editor, "\x1b[D", 3);
    assert(editor.cursor_y == 0 && editor.cursor_x == 2);

    editor_free(&editor);
}

static void test_navigation_clamps_on_shorter_row(void)
{
    Editor editor;

    editor_init(&editor);
    editor_insert_row(&editor, 0, "long", 4);
    editor_insert_row(&editor, 1, "x", 1);
    editor.cursor_x = 4;
    editor.cursor_y = 0;

    process_key_bytes(&editor, "\x1b[B", 3);
    assert(editor.cursor_y == 1);
    assert(editor.cursor_x == 1);

    process_key_bytes(&editor, "\x1b[C", 3);
    assert(editor.cursor_y == 1);
    assert(editor.cursor_x == 1);

    editor_free(&editor);
}

static void test_home_and_end_keys(void)
{
    Editor editor;

    editor_init(&editor);
    editor_insert_row(&editor, 0, "hello", 5);
    editor.cursor_y = 0;
    editor.cursor_x = 2;

    process_key_bytes(&editor, "\x1b[F", 3); /* END */
    assert(editor.cursor_x == 5);
    process_key_bytes(&editor, "\x1b[H", 3); /* HOME */
    assert(editor.cursor_x == 0);

    editor_free(&editor);
}

static void test_page_up_and_down(void)
{
    Editor editor;
    int i;

    editor_init(&editor);
    editor.screen_rows = 2;
    for (i = 0; i < 6; i++) {
        editor_insert_row(&editor, i, "r", 1);
    }
    editor.cursor_y = 5;

    process_key_bytes(&editor, "\x1b[5~", 4); /* PAGE_UP */
    assert(editor.cursor_y == 3);
    process_key_bytes(&editor, "\x1b[6~", 4); /* PAGE_DOWN */
    assert(editor.cursor_y == 5);

    editor_free(&editor);
}

static void test_insert_newline_and_backspace_keys(void)
{
    Editor editor;

    editor_init(&editor);

    process_key_bytes(&editor, "a", 1);
    process_key_bytes(&editor, "b", 1);
    assert(strcmp(editor.rows[0].chars, "ab") == 0);

    process_key_bytes(&editor, "\r", 1);
    assert(editor.row_count == 2);

    process_key_bytes(&editor, "c", 1);
    assert(strcmp(editor.rows[1].chars, "c") == 0);

    process_key_bytes(&editor, "\x7f", 1); /* Backspace deletes 'c' */
    assert(strcmp(editor.rows[1].chars, "") == 0);

    process_key_bytes(&editor, "\x7f", 1); /* Backspace at col 0 joins rows */
    assert(editor.row_count == 1);
    assert(strcmp(editor.rows[0].chars, "ab") == 0);

    editor_free(&editor);
}

static void test_delete_key_within_row(void)
{
    Editor editor;

    editor_init(&editor);
    editor_insert_row(&editor, 0, "abc", 3);
    editor.cursor_y = 0;
    editor.cursor_x = 0;

    process_key_bytes(&editor, "\x1b[3~", 4); /* DELETE removes char under cursor */
    assert(strcmp(editor.rows[0].chars, "bc") == 0);
    assert(editor.cursor_x == 0);

    editor_free(&editor);
}

static void test_delete_key_at_end_joins_rows(void)
{
    Editor editor;

    editor_init(&editor);
    editor_insert_row(&editor, 0, "ab", 2);
    editor_insert_row(&editor, 1, "cd", 2);
    editor.cursor_y = 0;
    editor.cursor_x = 2;

    process_key_bytes(&editor, "\x1b[3~", 4); /* DELETE at line end pulls next row up */
    assert(editor.row_count == 1);
    assert(strcmp(editor.rows[0].chars, "abcd") == 0);

    editor_free(&editor);
}

static void test_quit_when_clean(void)
{
    Editor editor;

    editor_init(&editor);
    process_key_bytes(&editor, "\x11", 1); /* Ctrl-Q */
    assert(editor.should_quit == 1);
    editor_free(&editor);
}

static void test_dirty_quit_requires_confirmation(void)
{
    Editor editor;

    editor_init(&editor);
    editor.dirty = 1;

    process_key_bytes(&editor, "\x11", 1);
    assert(editor.should_quit == 0);
    assert(editor.quit_times == 2);
    assert(strstr(editor.status_message, "2 more times") != NULL);

    /* Any other key resets the quit counter. */
    process_key_bytes(&editor, "x", 1);
    assert(editor.quit_times == MiniEditor_QUIT_TIMES);

    process_key_bytes(&editor, "\x11", 1);
    process_key_bytes(&editor, "\x11", 1);
    process_key_bytes(&editor, "\x11", 1);
    assert(editor.should_quit == 1);

    editor_free(&editor);
}

int main(void)
{
    test_arrow_keys_navigate();
    test_arrow_wraps_between_rows();
    test_navigation_clamps_on_shorter_row();
    test_home_and_end_keys();
    test_page_up_and_down();
    test_insert_newline_and_backspace_keys();
    test_delete_key_within_row();
    test_delete_key_at_end_joins_rows();
    test_quit_when_clean();
    test_dirty_quit_requires_confirmation();
    return 0;
}
