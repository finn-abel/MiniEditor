# MiniEditor

MiniEditor is a small C11 terminal text editor. It runs in raw terminal mode,
draws with ANSI escape sequences, and aims for a practical Nano-style editing
experience without external UI dependencies.

It is intentionally simple: one file at a time, straightforward controls, line
numbers, a status/message bar, search, undo/redo, and basic C-family syntax
highlighting.

## Features

- Open an existing file or start a new unnamed buffer.
- Save files, including a `Save as` prompt for unnamed buffers.
- Insert text, split lines, join lines, and delete forward/backward.
- Move with arrow keys, Home/End, and Page Up/Page Down.
- Scroll vertically and horizontally as the cursor moves.
- Show line numbers, filename, line count, file type, and dirty/clean state.
- Warn before quitting with unsaved changes.
- Search with live match highlighting and arrow-key match navigation.
- Undo and redo up to 1000 editing operations.
- Highlight C-family files: `.c`, `.h`, `.cpp`, `.hpp`, and `.cc`.

## Build

Use the included Makefile:

```bash
make
```

This builds the `MiniEditor` executable with:

```text
-Wall -Wextra -Werror -std=c11 -g
```

To remove build artifacts:

```bash
make clean
```

## Run

Start with an empty buffer:

```bash
./MiniEditor
```

Open a file:

```bash
./MiniEditor path/to/file.c
```

If the file does not exist, MiniEditor starts a new buffer using that filename
and creates the file when you save.

## Controls

```text
Ctrl-S        Save
Ctrl-Q        Quit
Ctrl-F        Search
Ctrl-Z        Undo
Ctrl-Y        Redo
Esc           Cancel prompt/search
Enter         Insert newline / confirm prompt
Backspace     Delete before cursor
Delete        Delete under cursor
Arrow keys    Move cursor / navigate search matches
Home/End      Start/end of line
Page Up/Down  Move one screen
```

When a file has unsaved changes, `Ctrl-Q` asks for confirmation. Press
`Ctrl-Q` three times total to quit without saving.

## Testing

Run the automated tests with:

```bash
make test
```

The test target builds and runs the unit tests under `tests/`, then cleans the
generated test executables and object files.

For a full local check before finishing a change:

```bash
make clean
make
make test
```

## Project Layout

```text
include/      Public headers for editor modules
src/          Editor implementation
tests/        Unit tests for non-interactive behavior
test_files/   Small fixtures used by tests and manual checks
Makefile      Build, test, run, and clean targets
AGENT.md      Development guide and project scope notes
```

## Limitations

- Edits one file per process.
- No mouse support, tabs, split panes, plugins, config files, or LSP features.
- Syntax highlighting is currently limited to C-family file extensions.
- The editor depends on a POSIX-style terminal environment with ANSI escape
  sequence support.
- Terminal behavior is interactive, so raw-mode cleanup should be manually
  checked after terminal-related changes.
