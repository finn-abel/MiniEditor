#include "undo.h"

#include "buffer.h"
#include "editor.h"
#include "row.h"
#include "syntax.h"

#include <stdlib.h>
#include <string.h>

void undo_stack_init(UndoStack *stack)
{
    int i;

    stack->count = 0;
    stack->position = 0;
    stack->saved_position = 0;
    stack->locked = 0;
    for (i = 0; i < UNDO_MAX; i++) {
        stack->entries[i].text = NULL;
    }
}

void undo_stack_free(UndoStack *stack)
{
    int i;

    for (i = 0; i < stack->count; i++) {
        if (stack->entries[i].text != NULL) {
            free(stack->entries[i].text);
            stack->entries[i].text = NULL;
        }
    }
    stack->count = 0;
    stack->position = 0;
}

// Free redo entries and, if the saved position falls in the discarded range,
// mark it as unrecoverable (-1) so dirty tracking stays correct.
static void undo_discard_redo(UndoStack *stack)
{
    int i;

    for (i = stack->position; i < stack->count; i++) {
        if (stack->entries[i].text != NULL) {
            free(stack->entries[i].text);
            stack->entries[i].text = NULL;
        }
    }
    if (stack->saved_position > stack->position) {
        stack->saved_position = -1;
    }
    stack->count = stack->position;
}

// Drop the oldest entry to make room. Adjusts saved_position so
// position comparisons remain meaningful after the shift.
static void undo_evict_oldest(UndoStack *stack)
{
    if (stack->entries[0].text != NULL) {
        free(stack->entries[0].text);
    }
    memmove(&stack->entries[0], &stack->entries[1],
            sizeof(UndoEntry) * (UNDO_MAX - 1));
    stack->entries[UNDO_MAX - 1].text = NULL;
    stack->count--;
    stack->position--;
    if (stack->saved_position > 0) {
        stack->saved_position--;
    } else {
        stack->saved_position = -1;
    }
}

// Prepare the slot at position for a new entry. Returns a pointer to it.
static UndoEntry *undo_alloc_entry(UndoStack *stack)
{
    undo_discard_redo(stack);
    if (stack->count == UNDO_MAX) {
        undo_evict_oldest(stack);
    }
    return &stack->entries[stack->position];
}

static void undo_commit_entry(UndoStack *stack)
{
    stack->position++;
    stack->count = stack->position;
}

void undo_push_insert_char(UndoStack *stack, int cursor_y, int cursor_x,
                           int row_y, int col_x, char ch, int new_row)
{
    UndoEntry *e;

    if (stack->locked) {
        return;
    }
    e = undo_alloc_entry(stack);
    e->type = UNDO_INSERT_CHAR;
    e->cursor_y = cursor_y;
    e->cursor_x = cursor_x;
    e->row_y = row_y;
    e->col_x = col_x;
    e->ch = ch;
    e->text = NULL;
    e->text_len = 0;
    e->new_row = new_row;
    undo_commit_entry(stack);
}

void undo_push_delete_char(UndoStack *stack, int cursor_y, int cursor_x,
                           int row_y, int col_x, char ch)
{
    UndoEntry *e;

    if (stack->locked) {
        return;
    }
    e = undo_alloc_entry(stack);
    e->type = UNDO_DELETE_CHAR;
    e->cursor_y = cursor_y;
    e->cursor_x = cursor_x;
    e->row_y = row_y;
    e->col_x = col_x;
    e->ch = ch;
    e->text = NULL;
    e->text_len = 0;
    e->new_row = 0;
    undo_commit_entry(stack);
}

void undo_push_insert_newline(UndoStack *stack, int cursor_y, int cursor_x,
                              int row_y)
{
    UndoEntry *e;

    if (stack->locked) {
        return;
    }
    e = undo_alloc_entry(stack);
    e->type = UNDO_INSERT_NEWLINE;
    e->cursor_y = cursor_y;
    e->cursor_x = cursor_x;
    e->row_y = row_y;
    e->col_x = 0;
    e->ch = 0;
    e->text = NULL;
    e->text_len = 0;
    e->new_row = 0;
    undo_commit_entry(stack);
}

void undo_push_join_row(UndoStack *stack, int cursor_y, int cursor_x,
                        int row_y, int col_x, const char *text, int text_len)
{
    UndoEntry *e;
    char *text_copy;

    if (stack->locked) {
        return;
    }
    text_copy = malloc((size_t) text_len + 1);
    if (text_copy == NULL) {
        return;
    }
    memcpy(text_copy, text, (size_t) text_len);
    text_copy[text_len] = '\0';

    e = undo_alloc_entry(stack);
    e->type = UNDO_JOIN_ROW;
    e->cursor_y = cursor_y;
    e->cursor_x = cursor_x;
    e->row_y = row_y;
    e->col_x = col_x;
    e->ch = 0;
    e->text = text_copy;
    e->text_len = text_len;
    e->new_row = 0;
    undo_commit_entry(stack);
}

int undo_can_undo(UndoStack *stack)
{
    return stack->position > 0;
}

int undo_can_redo(UndoStack *stack)
{
    return stack->position < stack->count;
}

void undo_mark_saved(UndoStack *stack)
{
    stack->saved_position = stack->position;
}

// Set dirty: 0 when the stack position exactly matches the saved snapshot,
// 1 otherwise. saved_position of -1 means the saved state was evicted.
static void undo_update_dirty(struct Editor *editor)
{
    UndoStack *stack = &editor->undo_stack;

    if (stack->saved_position >= 0 &&
        stack->position == stack->saved_position) {
        editor->dirty = 0;
    } else {
        editor->dirty = 1;
    }
}

void undo_undo(struct Editor *editor)
{
    UndoStack *stack = &editor->undo_stack;
    UndoEntry *entry;
    EditorRow *row;

    if (!undo_can_undo(stack)) {
        return;
    }

    stack->position--;
    entry = &stack->entries[stack->position];
    stack->locked = 1;

    switch (entry->type) {
        case UNDO_INSERT_CHAR:
            if (entry->row_y >= 0 && entry->row_y < editor->row_count) {
                row = &editor->rows[entry->row_y];
                row_delete_char(row, entry->col_x);
                syntax_update(editor, row);
                if (entry->new_row) {
                    editor_delete_row(editor, entry->row_y);
                }
            }
            break;

        case UNDO_DELETE_CHAR:
            if (entry->row_y >= 0 && entry->row_y < editor->row_count) {
                row = &editor->rows[entry->row_y];
                row_insert_char(row, entry->col_x, entry->ch);
                syntax_update(editor, row);
            }
            break;

        case UNDO_INSERT_NEWLINE:
            // Rejoin: append the split content from row_y back to row_y-1,
            // then delete the now-redundant row at row_y.
            if (entry->row_y > 0 && entry->row_y < editor->row_count) {
                row_append_string(&editor->rows[entry->row_y - 1],
                                  editor->rows[entry->row_y].chars,
                                  (size_t) editor->rows[entry->row_y].size);
                syntax_update(editor, &editor->rows[entry->row_y - 1]);
            }
            if (entry->row_y >= 0 && entry->row_y < editor->row_count) {
                editor_delete_row(editor, entry->row_y);
            }
            break;

        case UNDO_JOIN_ROW:
            // Re-split: truncate row_y-1 back to the pre-join length,
            // then restore the deleted row at row_y with its original content.
            if (entry->row_y > 0 && (entry->row_y - 1) < editor->row_count) {
                row = &editor->rows[entry->row_y - 1];
                row->size = entry->col_x;
                row->chars[row->size] = '\0';
                row_update_render(row);
                syntax_update(editor, row);
            }
            editor_insert_row(editor, entry->row_y,
                              entry->text ? entry->text : "",
                              (size_t) entry->text_len);
            break;
    }

    editor->cursor_y = entry->cursor_y;
    editor->cursor_x = entry->cursor_x;
    stack->locked = 0;
    undo_update_dirty(editor);
}

void undo_redo(struct Editor *editor)
{
    UndoStack *stack = &editor->undo_stack;
    UndoEntry *entry;

    if (!undo_can_redo(stack)) {
        return;
    }

    entry = &stack->entries[stack->position];
    stack->locked = 1;

    // Re-run the original operation by restoring the pre-operation cursor and
    // calling the same buffer function that recorded this entry.
    switch (entry->type) {
        case UNDO_INSERT_CHAR:
            editor->cursor_y = entry->cursor_y;
            editor->cursor_x = entry->cursor_x;
            editor_insert_char(editor, entry->ch);
            break;

        case UNDO_DELETE_CHAR:
            editor->cursor_y = entry->cursor_y;
            editor->cursor_x = entry->cursor_x;
            editor_delete_char(editor);
            break;

        case UNDO_INSERT_NEWLINE:
            editor->cursor_y = entry->cursor_y;
            editor->cursor_x = entry->cursor_x;
            editor_insert_newline(editor);
            break;

        case UNDO_JOIN_ROW:
            editor->cursor_y = entry->cursor_y;
            editor->cursor_x = entry->cursor_x;
            editor_delete_char(editor);
            break;
    }

    stack->position++;
    stack->locked = 0;
    undo_update_dirty(editor);
}
