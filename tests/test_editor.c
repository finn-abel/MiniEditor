#include <assert.h>

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

    editor_free(&editor);
    assert(editor.initialized == 0);
    assert(editor.screen_needs_clear == 0);

    return 0;
}
