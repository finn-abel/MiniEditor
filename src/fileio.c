#define _POSIX_C_SOURCE 200809L

#include "fileio.h"

#include "buffer.h"
#include "prompt.h"
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

    free(editor->filename);
    editor->filename = filename_copy;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        status_set(editor, "New file: %s", filename);
        editor->dirty = 0;
        return;
    }

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

int fileio_save(Editor *editor)
{
    char *buf;
    char *prompt_filename;
    char *temp_filename;
    int len;
    FILE *fp;
    size_t written;
    size_t filename_len;

    if (editor->filename == NULL) {
        prompt_filename = prompt_read(editor, "Save as: %s");
        if (prompt_filename == NULL) {
            status_set(editor, "Save cancelled");
            return -1;
        }

        free(editor->filename);
        editor->filename = prompt_filename;
    }

    buf = editor_rows_to_string(editor, &len);
    if (buf == NULL) {
        status_set(editor, "Could not serialize buffer");
        return -1;
    }

    filename_len = strlen(editor->filename);
    temp_filename = malloc(filename_len + 5);
    if (temp_filename == NULL) {
        free(buf);
        status_set(editor, "Could not allocate save path");
        return -1;
    }
    memcpy(temp_filename, editor->filename, filename_len);
    memcpy(&temp_filename[filename_len], ".tmp", 5);

    fp = fopen(temp_filename, "wb");
    if (fp == NULL) {
        free(temp_filename);
        free(buf);
        status_set(editor, "Could not write %s", editor->filename);
        return -1;
    }

    written = fwrite(buf, 1, (size_t) len, fp);
    if (fclose(fp) != 0 || written != (size_t) len) {
        remove(temp_filename);
        free(temp_filename);
        free(buf);
        status_set(editor, "Could not write %s", editor->filename);
        return -1;
    }

    if (rename(temp_filename, editor->filename) != 0) {
        remove(temp_filename);
        free(temp_filename);
        free(buf);
        status_set(editor, "Could not save %s", editor->filename);
        return -1;
    }

    free(temp_filename);
    free(buf);
    editor->dirty = 0;
    status_set(editor, "%d bytes written to disk", len);
    return 0;
}
