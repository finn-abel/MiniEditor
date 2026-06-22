#include <stdio.h>

#include "editor.h"

int main(void)
{
    Editor editor;

    editor_init(&editor);
    puts("MiniEditor starting");
    editor_free(&editor);

    return 0;
}
