#ifndef ROW_H
#define ROW_H

#include <stddef.h>

#define MiniEditor_TAB_STOP 8

/*
 * Stores one editable line and its terminal-ready rendered form.
 * The render buffer expands tabs so cursor math can use visible columns.
 */
typedef struct EditorRow {
    int index;
    int size;
    char *chars;
    int render_size;
    char *render;
    unsigned char *highlight;
} EditorRow;

/*
 * Initializes a row from a byte range.
 * The row owns copied character, render, and highlight buffers afterward.
 */
void row_init(EditorRow *row, int index, const char *s, size_t len);

/*
 * Releases all memory owned by a row.
 * The row should be initialized again before it is reused.
 */
void row_free(EditorRow *row);

/*
 * Rebuilds the rendered row from raw characters.
 * Tabs are expanded using MiniEditor_TAB_STOP columns.
 */
void row_update_render(EditorRow *row);

/*
 * Converts a raw character cursor position to a rendered column.
 * Tabs expand to the next configured tab stop while counting.
 */
int row_cursor_x_to_render_x(EditorRow *row, int cursor_x);

/*
 * Converts a rendered column back to the nearest raw cursor position.
 * This is used when moving through rows that contain expanded tabs.
 */
int row_render_x_to_cursor_x(EditorRow *row, int render_x);

#endif
