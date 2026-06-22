#include "terminal.h"

#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

// Save the user's terminal settings, then switch stdin into a byte-at-a-time
// mode suitable for an editor command loop.
int terminal_enable_raw_mode(Editor *editor)
{
    struct termios raw;

    if (tcgetattr(STDIN_FILENO, &editor->original_termios) == -1) {
        return -1;
    }

    raw = editor->original_termios;
    raw.c_iflag &= (tcflag_t) ~(ICRNL | IXON);
    raw.c_lflag &= (tcflag_t) ~(ECHO | ICANON | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        return -1;
    }

    editor->raw_mode_enabled = 1;
    return 0;
}

// Restore the saved terminal settings if this editor session successfully
// enabled raw mode. This function is intentionally safe to call more than once.
void terminal_disable_raw_mode(Editor *editor)
{
    if (editor->raw_mode_enabled) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &editor->original_termios);
        editor->raw_mode_enabled = 0;
    }
}

// Read exactly one byte from stdin. Interrupted reads are retried so Ctrl-C and
// similar signals do not leave the caller with a partial key result.
int terminal_read_key(void)
{
    unsigned char key;
    ssize_t bytes_read;

    do {
        bytes_read = read(STDIN_FILENO, &key, 1);
    } while (bytes_read == -1 && errno == EINTR);

    if (bytes_read != 1) {
        return -1;
    }

    return key;
}

// Ask the terminal driver for the current visible size. The editor reserves
// status/message rows after this function returns the real terminal dimensions.
int terminal_get_window_size(int *rows, int *cols)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    }

    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
}
