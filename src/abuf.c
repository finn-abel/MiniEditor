#include "abuf.h"

#include <stdlib.h>
#include <string.h>

// Grow the buffer just enough for the new bytes, then copy them after the
// existing content. Callers decide what those bytes mean.
void abuf_append(AppendBuffer *ab, const char *s, int len)
{
    char *new_data;

    if (len <= 0) {
        return;
    }

    new_data = realloc(ab->data, (size_t) ab->len + (size_t) len);
    if (new_data == NULL) {
        return;
    }

    memcpy(&new_data[ab->len], s, (size_t) len);
    ab->data = new_data;
    ab->len += len;
}

// Reset the small append buffer back to its empty state after a frame is
// written or a test is finished inspecting it.
void abuf_free(AppendBuffer *ab)
{
    free(ab->data);
    ab->data = NULL;
    ab->len = 0;
}
