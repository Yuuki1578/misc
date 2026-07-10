#define MISC_IMPL
#include "../misc.h"

uint8_t stackBuffer[1024 + sizeof(Arena)];
uint8_t copyBuffer[1024 + sizeof(Arena)];

int main(void)
{
    Arena *arena = CreateArena(sizeof stackBuffer, MISC_ARSTACK | MISC_ARNOGROW, stackBuffer);
    assert(arena != NULL);

    void *buf = ArenaAlloc(arena, 128);
    assert(buf != NULL);
    memset(buf, 100, 128);

    buf = ArenaAlloc(arena, 128);
    assert(buf != NULL);
    memset(buf, 200, 128);

    buf = ArenaAlloc(arena, 128);
    assert(buf != NULL);
    memset(buf, 'A', 128);

    buf = ArenaAlloc(arena, 1024);
    assert(buf == NULL);

    DestroyArena(arena);
    assert(memcmp(stackBuffer, copyBuffer, sizeof stackBuffer) == 0);
}
