#ifndef SEARCH_H
#define SEARCH_H

#include "editor.h"

/*
 * Opens an interactive search prompt.
 * Matches move the cursor, temporary highlights mark the active result.
 */
void search_find(Editor *editor);

#endif
