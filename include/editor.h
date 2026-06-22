#ifndef EDITOR_H
#define EDITOR_H

/*
 * Stores the shared state for one MiniEditor session.
 * The full editor state will be added as the implementation grows.
 */
typedef struct Editor {
    int initialized;
} Editor;

/*
 * Initializes an Editor instance into a known empty state.
 * The caller owns the Editor storage and passes it to editor_free later.
 */
void editor_init(Editor *editor);

/*
 * Releases resources owned by an Editor instance.
 * This early implementation has no dynamic resources yet.
 */
void editor_free(Editor *editor);

#endif
