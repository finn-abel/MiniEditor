#include "editor.h"

void editor_init(Editor *editor)
{
    editor->initialized = 1;
}

void editor_free(Editor *editor)
{
    editor->initialized = 0;
}
