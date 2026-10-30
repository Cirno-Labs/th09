#include "enemy.h"

void EclManager::Unload()
{
    if (this->file != NULL)
    {
        g_MemoryManager.Free(this->file);
    }
    this->file = NULL;
}
