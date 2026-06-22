CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -g
INCLUDES = -Iinclude

TARGET = MiniEditor

# Add normal project source files here.
SRC = src/main.c src/editor.c src/terminal.c src/abuf.c src/render.c src/input.c src/row.c src/buffer.c src/fileio.c src/status.c

# Add test source files here.
TEST_SRC = tests/test_editor.c tests/test_abuf.c tests/test_render.c tests/test_terminal.c tests/test_input.c tests/test_row.c tests/test_buffer.c tests/test_fileio.c tests/test_status.c

OBJ = $(SRC:.c=.o)

# Source files needed for tests should not include src/main.c,
# because each test file has its own main function.
TEST_SUPPORT_SRC = $(filter-out src/main.c, $(SRC))
TEST_SUPPORT_OBJ = $(TEST_SUPPORT_SRC:.c=.o)

TEST_TARGETS = $(TEST_SRC:tests/%.c=%)
TEST_OBJ = $(TEST_SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%: tests/%.o $(TEST_SUPPORT_OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

test: $(TEST_TARGETS)
	@if [ -z "$(TEST_TARGETS)" ]; then \
		echo "No tests exist yet."; \
	else \
		set -e; \
		for test in $(TEST_TARGETS); do \
			echo "Running $$test..."; \
			./$$test; \
			echo ""; \
		done; \
	fi
	$(MAKE) clean

clean:
	rm -f $(OBJ) $(TEST_SUPPORT_OBJ) $(TEST_OBJ) $(TARGET) $(TEST_TARGETS)
	rm -rf *.dSYM tests/*.dSYM

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run test
