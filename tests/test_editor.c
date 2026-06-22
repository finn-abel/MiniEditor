#include <assert.h>

#include "editor.h"

int main(void)
{
    Editor editor;

    editor_init(&editor);
    assert(editor.initialized == 1);

    editor_free(&editor);
    assert(editor.initialized == 0);

    return 0;
}
