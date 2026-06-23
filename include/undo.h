#ifndef UNDO_H
#define UNDO_H

/*
 * Operation types recorded in the undo stack.
 * Each type corresponds to one atomic editing action.
 */
typedef enum {
    UNDO_INSERT_CHAR,
    UNDO_DELETE_CHAR,
    UNDO_INSERT_NEWLINE,
    UNDO_JOIN_ROW,
} UndoType;

/*
 * Snapshot of one atomic editing operation.
 * Stores enough data to reverse or replay the action
 * and to restore the cursor to its pre-operation position.
 */
typedef struct {
    UndoType type;
    int cursor_y;
    int cursor_x;
    int row_y;
    int col_x;
    char ch;
    char *text;
    int text_len;
    int new_row;
} UndoEntry;

#define UNDO_MAX 1000

/*
 * Bounded undo/redo stack embedded in the Editor.
 * entries[0..position-1] is undoable history.
 * entries[position..count-1] is redoable history.
 * When locked, push calls are silently ignored to prevent
 * undo/redo operations from recording themselves.
 */
typedef struct {
    UndoEntry entries[UNDO_MAX];
    int count;
    int position;
    int saved_position;
    int locked;
} UndoStack;

/* Forward declaration so undo.h does not depend on editor.h. */
struct Editor;

/*
 * Initializes the undo stack to an empty state.
 * Must be called before any other undo function.
 */
void undo_stack_init(UndoStack *stack);

/*
 * Releases any heap memory owned by undo entries (JOIN_ROW text).
 * Safe to call on an already-freed or uninitialized stack.
 */
void undo_stack_free(UndoStack *stack);

/*
 * Records a character insertion at (row_y, col_x).
 * new_row is 1 when editor_insert_char auto-created an empty row first.
 */
void undo_push_insert_char(UndoStack *stack, int cursor_y, int cursor_x,
                           int row_y, int col_x, char ch, int new_row);

/*
 * Records a character deletion at (row_y, col_x).
 * ch is the character that was removed.
 */
void undo_push_delete_char(UndoStack *stack, int cursor_y, int cursor_x,
                           int row_y, int col_x, char ch);

/*
 * Records a newline insertion that created a new row at row_y.
 * cursor_y and cursor_x are the positions before the operation.
 */
void undo_push_insert_newline(UndoStack *stack, int cursor_y, int cursor_x,
                              int row_y);

/*
 * Records a row-join backspace at the start of row_y.
 * col_x is the original length of the previous row (the join point).
 * text is copied from the deleted row's content.
 */
void undo_push_join_row(UndoStack *stack, int cursor_y, int cursor_x,
                        int row_y, int col_x, const char *text, int text_len);

/*
 * Returns 1 if there is at least one operation to undo.
 */
int undo_can_undo(UndoStack *stack);

/*
 * Returns 1 if there is at least one operation to redo.
 */
int undo_can_redo(UndoStack *stack);

/*
 * Reverses the most recent operation and restores the cursor.
 * No-op when the stack is empty. Updates editor->dirty.
 */
void undo_undo(struct Editor *editor);

/*
 * Replays the next undone operation and restores the cursor.
 * No-op when there is nothing to redo. Updates editor->dirty.
 */
void undo_redo(struct Editor *editor);

/*
 * Marks the current stack position as the saved state.
 * editor->dirty will be 0 when position matches this value.
 */
void undo_mark_saved(UndoStack *stack);

#endif
