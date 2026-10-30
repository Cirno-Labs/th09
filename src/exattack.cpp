#include "exattack.h"
#include "gamemanager.h"

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

int ExAttackManager::AddedCallback(ExAttackManager *mgr)
{
    mgr->unknown14 = g_Unknown4a117c[g_GameManager.sides[0].character] * g_GameManager.difficulty +
                     g_Unknown4a1128[g_GameManager.sides[0].character];
    mgr->unknown18 = g_Unknown4a117c[g_GameManager.sides[1].character] * g_GameManager.difficulty +
                     g_Unknown4a1128[g_GameManager.sides[1].character];
    mgr->Reset();
    return 0;
}

ExAttackManager *ExAttackManager::RegisterChain()
{
    ExAttackManager *mgr = (ExAttackManager *)g_MemoryManager.Register(new ExAttackManager(), sizeof(ExAttackManager),
                                                                       "ExAttackCtrlInf");

    memset(mgr, 0, sizeof(ExAttackManager));
    mgr->calcElem = g_Chain.CreateElem((ChainCallback)ExAttackManager::OnUpdate);
    mgr->calcElem->arg = mgr;
    mgr->calcElem->addedCallback = (ChainCallback)ExAttackManager::AddedCallback;
    if (g_Chain.AddToCalcChain(mgr->calcElem, 0xe) != 0)
    {
        return NULL;
    }
    mgr->drawElem = g_Chain.CreateElem((ChainCallback)ExAttackManager::OnDraw);
    mgr->drawElem->arg = mgr;
    g_Chain.AddToDrawChain(mgr->drawElem, 0x10);
    mgr->drawElem2 = g_Chain.CreateElem((ChainCallback)ExAttackManager::OnDraw2);
    mgr->drawElem2->arg = mgr;
    g_Chain.AddToDrawChain(mgr->drawElem2, 0x1d);
    return mgr;
}
