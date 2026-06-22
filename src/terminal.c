#include "terminal.h"

#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

static int terminal_read_byte(void)
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

// Leave the terminal in a visually clean state before handing control back to
// the user's shell.
void terminal_clear_screen(void)
{
    (void) write(STDOUT_FILENO, "\x1b[2J", 4);
    (void) write(STDOUT_FILENO, "\x1b[H", 3);
}

// Read exactly one byte from stdin. Interrupted reads are retried so Ctrl-C and
// similar signals do not leave the caller with a partial key result.
int terminal_read_key(void)
{
    int key = terminal_read_byte();
    int seq0;
    int seq1;
    int seq2;

    if (key == -1) {
        return -1;
    }

    // Decode common ANSI escape sequences emitted by arrow and navigation
    // keys. Unknown sequences collapse back to Escape for now.
    if (key == '\x1b') {
        seq0 = terminal_read_byte();
        if (seq0 == -1) {
            return '\x1b';
        }

        seq1 = terminal_read_byte();
        if (seq1 == -1) {
            return '\x1b';
        }

        if (seq0 == '[') {
            if (seq1 >= '0' && seq1 <= '9') {
                seq2 = terminal_read_byte();
                if (seq2 == '~') {
                    switch (seq1) {
                        case '1':
                        case '7':
                            return HOME_KEY;
                        case '3':
                            return DELETE_KEY;
                        case '4':
                        case '8':
                            return END_KEY;
                        case '5':
                            return PAGE_UP;
                        case '6':
                            return PAGE_DOWN;
                        default:
                            break;
                    }
                }
            } else {
                switch (seq1) {
                    case 'A':
                        return ARROW_UP;
                    case 'B':
                        return ARROW_DOWN;
                    case 'C':
                        return ARROW_RIGHT;
                    case 'D':
                        return ARROW_LEFT;
                    case 'H':
                        return HOME_KEY;
                    case 'F':
                        return END_KEY;
                    default:
                        break;
                }
            }
        } else if (seq0 == 'O') {
            switch (seq1) {
                case 'H':
                    return HOME_KEY;
                case 'F':
                    return END_KEY;
                default:
                    break;
            }
        }

        return '\x1b';
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
