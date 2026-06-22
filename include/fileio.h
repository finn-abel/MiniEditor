#ifndef FILEIO_H
#define FILEIO_H

#include "editor.h"

/*
 * Opens a text file and loads each line into the editor buffer.
 * The editor stores a copy of the filename and marks the buffer clean.
 */
void fileio_open(Editor *editor, const char *filename);

#endif
