#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>

#include "editor.h"

/*
 * Inserts a new row into the editor at the requested index.
 * Rows at and after the insertion point are shifted down and reindexed.
 */
void editor_insert_row(Editor *editor, int at, const char *s, size_t len);

/*
 * Deletes a row from the editor at the requested index.
 * Remaining rows are shifted up and reindexed.
 */
void editor_delete_row(Editor *editor, int at);

#endif
