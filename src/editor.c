#include "editor.h"

// Set defaults that let small unit tests use an Editor without requiring a
// real terminal. main.c replaces the screen size after raw mode is enabled.
void editor_init(Editor *editor)
{
    editor->initialized = 1;
    editor->raw_mode_enabled = 0;
    editor->should_quit = 0;
    editor->screen_needs_clear = 1;
    editor->cursor_x = 0;
    editor->cursor_y = 0;
    editor->screen_rows = 22;
    editor->screen_cols = 80;
}

// Later modules will release owned editor state here. For now, this marks the
// early Editor object as no longer active.
void editor_free(Editor *editor)
{
    editor->initialized = 0;
    editor->raw_mode_enabled = 0;
    editor->should_quit = 0;
    editor->screen_needs_clear = 0;
}
