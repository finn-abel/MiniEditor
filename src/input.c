#include "input.h"

#include "terminal.h"

#define CTRL_KEY(key) ((key) & 0x1f)

static int input_max_cursor_x(Editor *editor)
{
    if (editor->cursor_y >= 0 && editor->cursor_y < editor->row_count) {
        return editor->rows[editor->cursor_y].size;
    }

    if (editor->screen_cols <= 0) {
        return 0;
    }

    return editor->screen_cols - 1;
}

static int input_max_cursor_y(Editor *editor)
{
    if (editor->row_count > 0) {
        return editor->row_count - 1;
    }

    if (editor->screen_rows <= 0) {
        return 0;
    }

    return editor->screen_rows - 1;
}

// Move within the currently visible editable area. File rows will eventually
// provide tighter line-specific limits, but the screen bounds are enough now.
static void input_move_cursor(Editor *editor, int key)
{
    switch (key) {
        case ARROW_LEFT:
            if (editor->cursor_x > 0) {
                editor->cursor_x--;
            }
            break;
        case ARROW_RIGHT:
            if (editor->cursor_x < input_max_cursor_x(editor)) {
                editor->cursor_x++;
            }
            break;
        case ARROW_UP:
            if (editor->cursor_y > 0) {
                editor->cursor_y--;
            }
            break;
        case ARROW_DOWN:
            if (editor->cursor_y < input_max_cursor_y(editor)) {
                editor->cursor_y++;
            }
            break;
        default:
            break;
    }

    if (editor->cursor_x > input_max_cursor_x(editor)) {
        editor->cursor_x = input_max_cursor_x(editor);
    }
}

// Dispatch one decoded key. At this stage, only quitting and basic navigation
// mutate editor state.
void input_process_keypress(Editor *editor)
{
    int key = terminal_read_key();

    switch (key) {
        case CTRL_KEY('q'):
            editor->should_quit = 1;
            break;
        case HOME_KEY:
            editor->cursor_x = 0;
            break;
        case END_KEY:
            editor->cursor_x = input_max_cursor_x(editor);
            break;
        case PAGE_UP:
            editor->cursor_y -= editor->screen_rows;
            if (editor->cursor_y < 0) {
                editor->cursor_y = 0;
            }
            break;
        case PAGE_DOWN:
            editor->cursor_y += editor->screen_rows;
            if (editor->cursor_y > input_max_cursor_y(editor)) {
                editor->cursor_y = input_max_cursor_y(editor);
            }
            break;
        case ARROW_LEFT:
        case ARROW_RIGHT:
        case ARROW_UP:
        case ARROW_DOWN:
            input_move_cursor(editor, key);
            break;
        default:
            break;
    }

    if (editor->cursor_x > input_max_cursor_x(editor)) {
        editor->cursor_x = input_max_cursor_x(editor);
    }
}
