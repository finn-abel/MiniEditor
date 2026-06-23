#ifndef SYNTAX_H
#define SYNTAX_H

#include "editor.h"

#define HL_HIGHLIGHT_NUMBERS (1 << 0)
#define HL_HIGHLIGHT_STRINGS (1 << 1)

/*
 * Describes one supported language syntax.
 * File matches select the syntax and keyword suffixes encode keyword groups.
 */
typedef struct EditorSyntax {
    const char *filetype;
    const char **filematch;
    const char **keywords;
    const char *singleline_comment_start;
    const char *multiline_comment_start;
    const char *multiline_comment_end;
    int flags;
} EditorSyntax;

/*
 * Selects syntax highlighting based on the editor filename.
 * All rows are refreshed after the selected syntax changes.
 */
void syntax_select(Editor *editor);

/*
 * Updates the highlight array for one row.
 * C-style highlighting includes comments, strings, numbers, and keywords.
 */
void syntax_update(Editor *editor, EditorRow *row);

/*
 * Returns the ANSI color code for one highlight type.
 * Render code asks this function instead of hardcoding language colors.
 */
int syntax_color(int highlight);

#endif
