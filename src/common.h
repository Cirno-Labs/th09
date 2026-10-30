// Shared helpers for the decompiled code.

#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Fails to compile unless sizeof(type) == size. Generates no code.
#define ASSERT_SIZE(type, size) typedef char assert_size_##type[sizeof(type) == (size) ? 1 : -1]

// The number of elements of array `a`, as an unsigned size_t.
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

// The object every allocation goes through, as th08's (at 0x4ad018).
class MemoryManager
{
public:
    // (0x42b0d0)
    MemoryManager();
    // malloc() and free(); `name` is unused in the release build
    // (0x401340, 0x401360).
    void *Alloc(int size, const char *name);
    void Free(void *p);
    // Records `p`, a block of `size` bytes allocated with new, under
    // `name` and returns it; a no-op in the release build (0x401380).
    void *Register(void *p, int size, const char *name);

    // The blocks registered in the debug build.
    void *blocks[0x1000];
    // Set to have the destructor free the blocks.
    int freeBlocksOnExit;
};
ASSERT_SIZE(MemoryManager, 0x4004);
extern MemoryManager g_MemoryManager;
