#ifndef INPUT_H
#define INPUT_H

#include "editor.h"

/*
 * Reads and handles one keypress for the active editor session.
 * This owns command dispatch, including cursor movement and quitting.
 */
void input_process_keypress(Editor *editor);

#endif
