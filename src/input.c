#include "input.h"

#include "buffer.h"
#include "fileio.h"
#include "search.h"
#include "status.h"
#include "terminal.h"

#define CTRL_KEY(key) ((key) & 0x1f)

static int input_max_cursor_x(Editor *editor)
{
    if (editor->cursor_y >= 0 && editor->cursor_y < editor->row_count) {
        return editor->rows[editor->cursor_y].size;
    }

    return 0;
}

static int input_max_cursor_y(Editor *editor)
{
    if (editor->row_count > 0) {
        return editor->row_count - 1;
    }

    return 0;
}

static void input_clamp_cursor(Editor *editor)
{
    if (editor->cursor_y < 0) {
        editor->cursor_y = 0;
    }
    if (editor->cursor_y > input_max_cursor_y(editor)) {
        editor->cursor_y = input_max_cursor_y(editor);
    }
    if (editor->cursor_x < 0) {
        editor->cursor_x = 0;
    }
    if (editor->cursor_x > input_max_cursor_x(editor)) {
        editor->cursor_x = input_max_cursor_x(editor);
    }
}

// Move within the currently visible editable area. File rows will eventually
// provide tighter line-specific limits, but the screen bounds are enough now.
static void input_move_cursor(Editor *editor, int key)
{
    switch (key) {
        case ARROW_LEFT:
            if (editor->cursor_x > 0) {
                editor->cursor_x--;
            } else if (editor->cursor_y > 0) {
                editor->cursor_y--;
                editor->cursor_x = input_max_cursor_x(editor);
            }
            break;
        case ARROW_RIGHT:
            if (editor->cursor_x < input_max_cursor_x(editor)) {
                editor->cursor_x++;
            } else if (editor->cursor_y < input_max_cursor_y(editor)) {
                editor->cursor_y++;
                editor->cursor_x = 0;
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

    input_clamp_cursor(editor);
}

static void input_delete_forward(Editor *editor)
{
    if (editor->cursor_y >= editor->row_count) {
        return;
    }

    if (editor->cursor_x < input_max_cursor_x(editor)) {
        editor->cursor_x++;
        editor_delete_char(editor);
    } else if (editor->cursor_y < input_max_cursor_y(editor)) {
        editor->cursor_y++;
        editor->cursor_x = 0;
        editor_delete_char(editor);
    }
}

// Dispatch one decoded key. At this stage, only quitting and basic navigation
// mutate editor state.
void input_process_keypress(Editor *editor)
{
    int key = terminal_read_key();
    int reset_quit_times = 1;

    switch (key) {
        case CTRL_KEY('q'):
            if (editor->dirty && editor->quit_times > 1) {
                status_set(editor,
                           "WARNING: file has unsaved changes. Press Ctrl-Q %d more times to quit.",
                           editor->quit_times - 1);
                editor->quit_times--;
                reset_quit_times = 0;
                break;
            }

            editor->should_quit = 1;
            reset_quit_times = 0;
            break;
        case CTRL_KEY('s'):
            fileio_save(editor);
            break;
        case CTRL_KEY('f'):
            search_find(editor);
            break;
        case '\r':
        case '\n':
            editor_insert_newline(editor);
            break;
        case 127:
        case CTRL_KEY('h'):
            editor_delete_char(editor);
            break;
        case DELETE_KEY:
            input_delete_forward(editor);
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
            if (key >= 32 && key <= 126) {
                editor_insert_char(editor, key);
            }
            break;
    }

    input_clamp_cursor(editor);

    if (reset_quit_times) {
        editor->quit_times = MiniEditor_QUIT_TIMES;
    }
}
