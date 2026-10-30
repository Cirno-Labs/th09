#include "enemy.h"
#include "gamemanager.h"
#include "sprite.h"
#include "unsorted.h"

void Enemy::FreeUnknown33d8()
{
    int i;

    for (i = 0; i < 4; i++)
    {
        if (this->unknown33d8[i] != NULL)
        {
            g_MemoryManager.Free(this->unknown33d8[i]);
            this->unknown33d8[i] = NULL;
        }
    }
}

// Frees the enemies' blocks, enemy.anm (slot 9) and the ECL files.
// Returns 0. Static, with the manager in ebx (0x411f30).
static int FreeResources(EnemyManager *mgr)
{
    int i;
    Enemy *enemy;

    enemy = mgr->enemies;
    for (i = 0; i < 0x80; i++, enemy++)
    {
        enemy->FreeUnknown33d8();
    }
    g_SpriteEngine->ReleaseAnm(9);
    mgr->ecl.Unload();
    mgr->playerEcl.Unload();
    return 0;
}

void EnemyManager::RemoveChain(EnemyManager *mgr)
{
    if (mgr != NULL)
    {
        FreeResources(mgr);
        g_Chain.Remove(mgr->calcElem);
        g_Chain.Remove(mgr->drawElem);
        g_Chain.Remove(mgr->drawElem2);
        delete mgr;
    }
}

void EnemyManager::Reset()
{
    int i;
    Enemy *enemy;

    enemy = this->enemies;
    for (i = 0; i < 0x80; i++, enemy++)
    {
        enemy->FreeUnknown33d8();
        enemy->active = 0;
    }
    if (this->sideIdx == 0)
    {
        for (i = 0; i < 0x100; i++)
        {
            g_TimelineOrder[i] = g_Rng.GetRandomU32InRange(this->ecl.GetTimelineCount());
            g_TimelineOrder[i] |= g_Rng.GetRandomU32InRange(2) << 7;
        }
    }
    this->unknown2ac3d4 = 0;
    this->timeline = this->ecl.GetTimeline(g_TimelineOrder[0] & 0x7f);
    this->timelineTimers.unknownC = 200;
    this->timelineTimers.unknown0 = 0;
}

EnemyManager::EnemyManager()
{
    this->Initialize();
}

EnemyManager *EnemyManager::RegisterChain(int sideIdx)
{
    EnemyManager *mgr =
        (EnemyManager *)g_MemoryManager.Register(new EnemyManager(), sizeof(EnemyManager), "EnemyCtrlInf");

    mgr->Initialize();
    mgr->sideIdx = sideIdx;
    mgr->side = &g_GameManager.sides[sideIdx];
    mgr->otherSide = &g_GameManager.sides[1 - sideIdx];
    mgr->calcElem = g_Chain.CreateElem((ChainCallback)EnemyManager::OnUpdate);
    mgr->calcElem->arg = mgr;
    mgr->calcElem->addedCallback = (ChainCallback)EnemyManager::AddedCallback;
    if (g_Chain.AddToCalcChain(mgr->calcElem, sideIdx + 0xa) != 0)
    {
        return NULL;
    }
    mgr->drawElem = g_Chain.CreateElem((ChainCallback)EnemyManager::OnDraw);
    mgr->drawElem->arg = mgr;
    g_Chain.AddToDrawChain(mgr->drawElem, sideIdx + 0xa);
    mgr->drawElem2 = g_Chain.CreateElem((ChainCallback)EnemyManager::OnDraw2);
    mgr->drawElem2->arg = mgr;
    g_Chain.AddToDrawChain(mgr->drawElem2, sideIdx + 0x11);
    return mgr;
}
