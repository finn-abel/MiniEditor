#include <assert.h>
#include <string.h>
#include <time.h>

#include "editor.h"
#include "status.h"

int main(void)
{
    Editor editor;
    time_t before;

    editor_init(&editor);
    before = time(NULL);
    status_set(&editor, "Loaded %d rows", 3);

    assert(strcmp(editor.status_message, "Loaded 3 rows") == 0);
    assert(editor.status_message_time >= before);

    editor_free(&editor);
    return 0;
}
