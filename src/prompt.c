#include "prompt.h"

#include "render.h"
#include "status.h"
#include "terminal.h"

#include <stdlib.h>
#include <string.h>

// Double the input buffer when the current prompt text fills it. If allocation
// fails, the old buffer stays valid and the caller keeps the existing capacity.
static int prompt_grow(char **buffer, int capacity)
{
    char *new_buffer = realloc(*buffer, (size_t) capacity * 2);

    if (new_buffer == NULL) {
        return capacity;
    }

    *buffer = new_buffer;
    return capacity * 2;
}

// Render the prompt in the message bar after every keypress so callers get a
// reusable inline input field without owning any terminal drawing details.
char *prompt_read(Editor *editor, const char *prompt)
{
    return prompt_read_with_callback(editor, prompt, NULL);
}

char *prompt_read_with_callback(Editor *editor, const char *prompt,
                                PromptCallback callback)
{
    int len = 0;
    int capacity = 32;
    char *buffer = malloc((size_t) capacity);

    if (buffer == NULL) {
        return NULL;
    }
    buffer[0] = '\0';

    while (1) {
        int key;

        status_set(editor, prompt, buffer);
        render_refresh_screen(editor);

        // Enter accepts non-empty input, Esc cancels, and Backspace edits the
        // in-progress response. Printable bytes are appended directly.
        key = terminal_read_key();
        if (key == '\r' || key == '\n') {
            if (len == 0) {
                free(buffer);
                status_set(editor, "");
                return NULL;
            }

            status_set(editor, "");
            return buffer;
        }

        if (key == '\x1b') {
            free(buffer);
            status_set(editor, "");
            return NULL;
        }

        if (key == 127 || key == ('h' & 0x1f)) {
            if (len > 0) {
                len--;
                buffer[len] = '\0';
            }
        } else if (key >= 32 && key <= 126) {
            if (len == capacity - 1) {
                capacity = prompt_grow(&buffer, capacity);
            }

            if (len < capacity - 1) {
                buffer[len] = (char) key;
                len++;
                buffer[len] = '\0';
            }
        }

        if (callback != NULL) {
            callback(editor, buffer, key);
        }
    }
}
