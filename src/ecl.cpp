#include "enemy.h"

void EclManager::Unload()
{
    if (this->eclFile != NULL)
    {
        g_MemoryManager.Free(this->eclFile);
    }
    this->eclFile = NULL;
}
