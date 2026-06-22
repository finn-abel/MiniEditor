#ifndef ABUF_H
#define ABUF_H

/*
 * Collects terminal output before writing it in one call.
 * This keeps full-screen redraws from flickering due to many small writes.
 */
typedef struct {
    char *data;
    int len;
} AppendBuffer;

/*
 * Appends a byte range to the append buffer.
 * If allocation fails, the buffer is left unchanged.
 */
void abuf_append(AppendBuffer *ab, const char *s, int len);

/*
 * Releases memory owned by the append buffer.
 * The buffer should not be reused unless reinitialized afterward.
 */
void abuf_free(AppendBuffer *ab);

#endif
