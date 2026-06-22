#ifndef STATUS_H
#define STATUS_H

#include "editor.h"

/*
 * Sets a temporary message for the editor message bar.
 * printf-style formatting is supported for short user-facing messages.
 */
void status_set(Editor *editor, const char *fmt, ...);

#endif
