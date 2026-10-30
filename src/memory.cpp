#include "common.h"

void *MemoryManager::Alloc(int size, const char *name)
{
    return malloc(size);
}

void MemoryManager::Free(void *p)
{
    free(p);
}
