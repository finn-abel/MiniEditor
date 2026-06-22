#ifndef PROMPT_H
#define PROMPT_H

#include "editor.h"

/*
 * Reads a short user response through the editor message bar.
 * Returns a heap string on Enter, or NULL if the user cancels with Esc.
 */
char *prompt_read(Editor *editor, const char *prompt);

#endif
