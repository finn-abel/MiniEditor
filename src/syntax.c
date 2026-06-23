#include "syntax.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// C-family syntax definition. A trailing '|' in the keyword table marks the
// second keyword color group.
static const char *c_filematch[] = {".c", ".h", ".cpp", ".hpp", ".cc", NULL};
static const char *c_keywords[] = {
    "switch", "if", "while", "for", "break", "continue", "return", "else",
    "struct", "union", "typedef", "static", "enum", "class", "case",
    "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
    "void|", "size_t|", "bool|", "const|", NULL};

static const EditorSyntax syntax_db[] = {
    {
        "C",
        c_filematch,
        c_keywords,
        "//",
        "/*",
        "*/",
        HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
    },
};

// Keep the syntax database as a plain static array for now. This helper avoids
// repeating the sizeof expression where we scan it.
static int syntax_db_entries(void)
{
    return (int) (sizeof(syntax_db) / sizeof(syntax_db[0]));
}

// Separators define token boundaries for keywords and numbers.
static int syntax_is_separator(int c)
{
    return isspace(c) || c == '\0' ||
           strchr(",.()+-/*=~%<>[];{}", c) != NULL;
}

static int syntax_file_matches(const char *filename, const char *filematch)
{
    size_t filename_len = strlen(filename);
    size_t filematch_len = strlen(filematch);

    if (filename_len < filematch_len) {
        return 0;
    }

    return strcmp(&filename[filename_len - filematch_len], filematch) == 0;
}

// Check whether a keyword starts at the current render position. Group 2
// keywords use the trailing '|' marker but the marker is not part of the token.
static int syntax_match_keyword(Editor *editor, EditorRow *row, int index,
                                int previous_separator, int *highlight)
{
    int keyword_index;

    if (!previous_separator || editor->syntax == NULL) {
        return 0;
    }

    for (keyword_index = 0; editor->syntax->keywords[keyword_index] != NULL;
         keyword_index++) {
        const char *keyword = editor->syntax->keywords[keyword_index];
        int keyword_len = (int) strlen(keyword);
        int keyword2 = keyword[keyword_len - 1] == '|';

        if (keyword2) {
            keyword_len--;
        }

        if (strncmp(&row->render[index], keyword, (size_t) keyword_len) == 0 &&
            syntax_is_separator(row->render[index + keyword_len])) {
            *highlight = keyword2 ? HL_KEYWORD2 : HL_KEYWORD1;
            return keyword_len;
        }
    }

    return 0;
}

// Pick a syntax based on filename suffix and refresh every row. Saving can
// change the filename, so this is called after both open and save.
void syntax_select(Editor *editor)
{
    int syntax_index;
    int row_index;

    editor->syntax = NULL;
    if (editor->filename == NULL) {
        return;
    }

    for (syntax_index = 0; syntax_index < syntax_db_entries(); syntax_index++) {
        const EditorSyntax *syntax = &syntax_db[syntax_index];
        int match_index;

        for (match_index = 0; syntax->filematch[match_index] != NULL;
             match_index++) {
            if (syntax_file_matches(editor->filename,
                                    syntax->filematch[match_index])) {
                editor->syntax = syntax;

                for (row_index = 0; row_index < editor->row_count; row_index++) {
                    syntax_update(editor, &editor->rows[row_index]);
                }
                return;
            }
        }
    }

    for (row_index = 0; row_index < editor->row_count; row_index++) {
        syntax_update(editor, &editor->rows[row_index]);
    }
}

// Scan one rendered row and write highlight types into the parallel highlight
// array. Multi-line comment state flows from the previous row to the next.
void syntax_update(Editor *editor, EditorRow *row)
{
    const char *scs;
    const char *mcs;
    const char *mce;
    int scs_len;
    int mcs_len;
    int mce_len;
    int previous_separator = 1;
    int in_string = 0;
    int in_comment;
    int index = 0;
    int previous_open_comment = row->highlight_open_comment;

    if (row->highlight == NULL) {
        return;
    }

    memset(row->highlight, HL_NORMAL, (size_t) row->render_size);
    row->highlight_open_comment = 0;

    if (editor->syntax == NULL) {
        return;
    }

    scs = editor->syntax->singleline_comment_start;
    mcs = editor->syntax->multiline_comment_start;
    mce = editor->syntax->multiline_comment_end;
    scs_len = scs != NULL ? (int) strlen(scs) : 0;
    mcs_len = mcs != NULL ? (int) strlen(mcs) : 0;
    mce_len = mce != NULL ? (int) strlen(mce) : 0;
    in_comment = row->index > 0 && editor->rows[row->index - 1].highlight_open_comment;

    while (index < row->render_size) {
        unsigned char previous_highlight =
            index > 0 ? row->highlight[index - 1] : HL_NORMAL;
        char c = row->render[index];

        if (scs_len > 0 && !in_string && !in_comment &&
            strncmp(&row->render[index], scs, (size_t) scs_len) == 0) {
            memset(&row->highlight[index], HL_COMMENT,
                   (size_t) (row->render_size - index));
            break;
        }

        if (mcs_len > 0 && mce_len > 0 && !in_string) {
            // While inside a block comment, everything stays comment-colored
            // until the configured closing marker is found.
            if (in_comment) {
                row->highlight[index] = HL_MLCOMMENT;
                if (strncmp(&row->render[index], mce, (size_t) mce_len) == 0) {
                    memset(&row->highlight[index], HL_MLCOMMENT,
                           (size_t) mce_len);
                    index += mce_len;
                    in_comment = 0;
                    previous_separator = 1;
                    continue;
                }
                index++;
                continue;
            }

            if (strncmp(&row->render[index], mcs, (size_t) mcs_len) == 0) {
                memset(&row->highlight[index], HL_MLCOMMENT, (size_t) mcs_len);
                index += mcs_len;
                in_comment = 1;
                continue;
            }
        }

        if ((editor->syntax->flags & HL_HIGHLIGHT_STRINGS) != 0) {
            if (in_string) {
                row->highlight[index] = HL_STRING;
                // Escaped characters stay inside the current string.
                if (c == '\\' && index + 1 < row->render_size) {
                    row->highlight[index + 1] = HL_STRING;
                    index += 2;
                    continue;
                }
                if (c == in_string) {
                    in_string = 0;
                }
                index++;
                previous_separator = 1;
                continue;
            }

            if (c == '"' || c == '\'') {
                in_string = c;
                row->highlight[index] = HL_STRING;
                index++;
                continue;
            }
        }

        if (c == '#' && previous_separator) {
            memset(&row->highlight[index], HL_PREPROCESSOR,
                   (size_t) (row->render_size - index));
            break;
        }

        if ((editor->syntax->flags & HL_HIGHLIGHT_NUMBERS) != 0) {
            if ((isdigit(c) && (previous_separator ||
                                previous_highlight == HL_NUMBER)) ||
                (c == '.' && previous_highlight == HL_NUMBER)) {
                row->highlight[index] = HL_NUMBER;
                index++;
                previous_separator = 0;
                continue;
            }
        }

        {
            int highlight = HL_NORMAL;
            int keyword_len = syntax_match_keyword(editor, row, index,
                                                   previous_separator,
                                                   &highlight);

            if (keyword_len > 0) {
                memset(&row->highlight[index], highlight, (size_t) keyword_len);
                index += keyword_len;
                previous_separator = 0;
                continue;
            }
        }

        previous_separator = syntax_is_separator(c);
        index++;
    }

    row->highlight_open_comment = in_comment;
    // If this row's block-comment state changed, the following row may need a
    // different starting state, so update forward until the state stabilizes.
    if (row->highlight_open_comment != previous_open_comment &&
        row->index + 1 < editor->row_count) {
        syntax_update(editor, &editor->rows[row->index + 1]);
    }
}

// Render asks this function for all syntax colors, keeping ANSI color choices
// centralized instead of scattered through drawing code.
int syntax_color(int highlight)
{
    switch (highlight) {
        case HL_COMMENT:
        case HL_MLCOMMENT:
            return 90;
        case HL_KEYWORD1:
            return 33;
        case HL_KEYWORD2:
            return 32;
        case HL_STRING:
            return 35;
        case HL_NUMBER:
            return 31;
        case HL_MATCH:
            return 34;
        case HL_PREPROCESSOR:
            return 36;
        default:
            return 39;
    }
}
