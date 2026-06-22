#include <stdio.h>

#include "buffer.h"
#include "editor.h"
#include "input.h"
#include "render.h"
#include "terminal.h"

// Keep startup simple: initialize state, enter raw mode, render, then process
// one key at a time until Ctrl-Q requests a normal shutdown.
int main(void)
{
    Editor editor;

    editor_init(&editor);
    editor_insert_row(&editor, 0, "int main(void) {", 16);

    if (terminal_enable_raw_mode(&editor) != 0) {
        perror("terminal_enable_raw_mode");
        editor_free(&editor);
        return 1;
    }

    if (terminal_get_window_size(&editor.screen_rows, &editor.screen_cols) != 0) {
        terminal_disable_raw_mode(&editor);
        editor_free(&editor);
        perror("terminal_get_window_size");
        return 1;
    }

    // The bottom two rows are reserved for the future status and message bars,
    // so render.c only draws editor rows above that area.
    if (editor.screen_rows > 2) {
        editor.screen_rows -= 2;
    }

    while (!editor.should_quit) {
        render_refresh_screen(&editor);
        input_process_keypress(&editor);
    }

    terminal_clear_screen();
    terminal_disable_raw_mode(&editor);
    editor_free(&editor);

    return 0;
}
