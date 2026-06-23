#include <assert.h>
#include <string.h>
#include <time.h>

#include "editor.h"
#include "status.h"

static void test_status_formats_message(void)
{
    Editor editor;
    time_t before;

    editor_init(&editor);
    before = time(NULL);
    status_set(&editor, "Loaded %d rows", 3);

    assert(strcmp(editor.status_message, "Loaded 3 rows") == 0);
    assert(editor.status_message_time >= before);

    editor_free(&editor);
}

static void test_status_overwrites_previous(void)
{
    Editor editor;

    editor_init(&editor);
    status_set(&editor, "first");
    status_set(&editor, "second %s", "message");
    assert(strcmp(editor.status_message, "second message") == 0);

    editor_free(&editor);
}

int main(void)
{
    test_status_formats_message();
    test_status_overwrites_previous();
    return 0;
}
