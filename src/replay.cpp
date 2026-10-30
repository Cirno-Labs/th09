#include "replay.h"

// Removes the chain elements unknownElem158 and unknownElem15c, if set,
// and frees the blocks. Returns 0. Static, with the manager in eax
// (0x420a60).
static int FreeResources(ReplayManager *mgr)
{
    if (mgr->unknownElem158 != NULL)
    {
        g_Chain.Remove(mgr->unknownElem158);
        mgr->unknownElem158 = NULL;
    }
    if (mgr->unknownElem15c != NULL)
    {
        g_Chain.Remove(mgr->unknownElem15c);
        mgr->unknownElem15c = NULL;
    }
    g_MemoryManager.Free(mgr->unknown08);
    if (mgr->unknown110 != NULL)
    {
        g_MemoryManager.Free(mgr->unknown110);
    }
    return 0;
}

void ReplayManager::RemoveChain(ReplayManager *mgr)
{
    if (mgr != NULL)
    {
        FreeResources(mgr);
        g_Chain.Remove(mgr->calcElem);
        delete mgr;
    }
}
