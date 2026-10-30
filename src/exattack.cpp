#include "exattack.h"

// Calls the onFree callback of each running attack and frees its block.
// Returns 0. Static, with the manager in eax (0x415650).
static int FreeAttacks(ExAttackManager *mgr)
{
    int i;
    ExAttack *attack;

    attack = mgr->attacks;
    for (i = 0; i < 0x100; i++, attack++)
    {
        if (attack->active == 0)
        {
            continue;
        }
        if (attack->onFree != NULL)
        {
            attack->onFree(attack);
        }
        if (attack->unknown1c != NULL)
        {
            g_MemoryManager.Free(attack->unknown1c);
            attack->unknown1c = NULL;
        }
    }
    return 0;
}

void ExAttackManager::RemoveChain(ExAttackManager *mgr)
{
    if (mgr != NULL)
    {
        FreeAttacks(mgr);
        g_Chain.Remove(mgr->drawElem2);
        g_Chain.Remove(mgr->drawElem);
        g_Chain.Remove(mgr->calcElem);
        delete mgr;
    }
}

void ExAttackManager::Reset()
{
    int i;
    ExAttack *attack;

    attack = this->attacks;
    for (i = 0; i < 0x100; i++, attack++)
    {
        if (attack->active == 0)
        {
            continue;
        }
        if (attack->onFree != NULL)
        {
            attack->onFree(attack);
        }
        if (attack->unknown1c != NULL)
        {
            g_MemoryManager.Free(attack->unknown1c);
            attack->unknown1c = NULL;
        }
        memset(attack, 0, sizeof(ExAttack));
    }
}
