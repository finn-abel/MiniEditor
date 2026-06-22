#include "status.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

void status_set(Editor *editor, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsnprintf(editor->status_message, sizeof(editor->status_message), fmt, args);
    va_end(args);

    editor->status_message_time = time(NULL);
}
