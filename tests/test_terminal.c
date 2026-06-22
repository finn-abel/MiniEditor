#include <assert.h>

#include "editor.h"
#include "terminal.h"

int main(void)
{
    Editor editor;
    int rows = -1;
    int cols = -1;
    int result;

    editor_init(&editor);
    terminal_disable_raw_mode(&editor);
    assert(editor.raw_mode_enabled == 0);

    result = terminal_get_window_size(&rows, &cols);
    if (result == 0) {
        assert(rows > 0);
        assert(cols > 0);
    } else {
        assert(result == -1);
        assert(rows == -1);
        assert(cols == -1);
    }

    editor_free(&editor);
    return 0;
}
