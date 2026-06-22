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

/*
 * Inserts one printable character at the editor cursor.
 * Missing rows are created as needed and the cursor moves right.
 */
void editor_insert_char(Editor *editor, int c);

/*
 * Inserts a newline at the cursor, splitting the current row when needed.
 * The cursor moves to the beginning of the newly created row.
 */
void editor_insert_newline(Editor *editor);

/*
 * Deletes the character before the cursor.
 * At the beginning of a row, it joins that row into the previous row.
 */
void editor_delete_char(Editor *editor);

/*
 * Serializes all editor rows into one heap buffer.
 * Rows are separated with newline bytes and the caller frees the result.
 */
char *editor_rows_to_string(Editor *editor, int *buf_len);

#endif
