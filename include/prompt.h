#ifndef PROMPT_H
#define PROMPT_H

#include "editor.h"

/*
 * Receives prompt updates after each processed key.
 * Search uses this to move between matches while the query is being edited.
 */
typedef void (*PromptCallback)(Editor *editor, const char *query, int key);

/*
 * Reads a short user response through the editor message bar.
 * Returns a heap string on Enter, or NULL if the user cancels with Esc.
 */
char *prompt_read(Editor *editor, const char *prompt);

/*
 * Reads prompt input and calls a callback after each key is handled.
 * The returned string follows the same ownership rules as prompt_read().
 */
char *prompt_read_with_callback(Editor *editor, const char *prompt,
                                PromptCallback callback);

#endif
