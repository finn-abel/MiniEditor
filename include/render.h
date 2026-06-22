#ifndef RENDER_H
#define RENDER_H

#include "editor.h"

/*
 * Redraws the visible editor screen from the current editor state.
 * The full frame is assembled first and then written to the terminal once.
 */
void render_refresh_screen(Editor *editor);

#endif
