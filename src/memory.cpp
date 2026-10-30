#include "common.h"

void *MemoryManager::Alloc(int size, const char *name)
{
    return malloc(size);
}

void MemoryManager::Free(void *p)
{
    free(p);
}

void *MemoryManager::Register(void *p, int size, const char *name)
{
    return p;
}
