#include <assert.h>
#include <string.h>

#include "abuf.h"

int main(void)
{
    AppendBuffer ab = {NULL, 0};

    abuf_append(&ab, "Mini", 4);
    assert(ab.len == 4);
    assert(memcmp(ab.data, "Mini", 4) == 0);

    abuf_append(&ab, "Editor", 6);
    assert(ab.len == 10);
    assert(memcmp(ab.data, "MiniEditor", 10) == 0);

    abuf_append(&ab, "ignored", 0);
    assert(ab.len == 10);

    abuf_free(&ab);
    assert(ab.data == NULL);
    assert(ab.len == 0);

    return 0;
}
