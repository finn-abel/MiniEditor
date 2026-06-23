#include <assert.h>
#include <string.h>

#include "abuf.h"

static void test_append_concatenates(void)
{
    AppendBuffer ab = {NULL, 0};

    abuf_append(&ab, "Mini", 4);
    assert(ab.len == 4);
    assert(memcmp(ab.data, "Mini", 4) == 0);

    abuf_append(&ab, "Editor", 6);
    assert(ab.len == 10);
    assert(memcmp(ab.data, "MiniEditor", 10) == 0);

    abuf_free(&ab);
}

static void test_append_ignores_non_positive_len(void)
{
    AppendBuffer ab = {NULL, 0};

    abuf_append(&ab, "ignored", 0);
    assert(ab.len == 0);
    assert(ab.data == NULL);

    abuf_append(&ab, "ignored", -5);
    assert(ab.len == 0);

    abuf_free(&ab);
}

static void test_free_resets_buffer(void)
{
    AppendBuffer ab = {NULL, 0};

    abuf_append(&ab, "data", 4);
    abuf_free(&ab);
    assert(ab.data == NULL);
    assert(ab.len == 0);

    /* Safe to free again on an already-empty buffer. */
    abuf_free(&ab);
    assert(ab.data == NULL);
}

int main(void)
{
    test_append_concatenates();
    test_append_ignores_non_positive_len();
    test_free_resets_buffer();
    return 0;
}
