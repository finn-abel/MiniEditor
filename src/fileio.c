#define _POSIX_C_SOURCE 200809L

#include "fileio.h"

#include "buffer.h"
#include "status.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *fileio_strdup(const char *s)
{
    size_t len = strlen(s);
    char *copy = malloc(len + 1);

    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, s, len + 1);
    return copy;
}

void fileio_open(Editor *editor, const char *filename)
{
    FILE *fp;
    char *line = NULL;
    size_t line_capacity = 0;
    ssize_t line_len;
    char *filename_copy;

    filename_copy = fileio_strdup(filename);
    if (filename_copy == NULL) {
        status_set(editor, "Could not copy filename");
        return;
    }

    fp = fopen(filename, "r");
    if (fp == NULL) {
        free(filename_copy);
        status_set(editor, "Could not open %s", filename);
        return;
    }

    free(editor->filename);
    editor->filename = filename_copy;

    // getline keeps blank lines and the final unterminated line, so loading
    // preserves the file shape before stripping newline characters.
    while ((line_len = getline(&line, &line_capacity, fp)) != -1) {
        while (line_len > 0 &&
               (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')) {
            line_len--;
        }

        editor_insert_row(editor, editor->row_count, line, (size_t) line_len);
    }

    free(line);
    fclose(fp);
    editor->dirty = 0;
}
