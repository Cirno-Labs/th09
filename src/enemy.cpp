#include "enemy.h"
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
        enemy->unknown337c &= ~1;
    }
    if (this->unknown31c == 0)
    {
        for (i = 0; i < 0x100; i++)
        {
            g_TimelineOrder[i] = g_Rng.GetRandomU32InRange(this->ecl.GetTimelineCount());
            g_TimelineOrder[i] |= g_Rng.GetRandomU32InRange(2) << 7;
        }
    }
    this->unknown2ac3d4 = 0;
    this->timeline = this->ecl.GetTimeline(g_TimelineOrder[0] & 0x7f);
    this->unknown2ac3ec = 200;
    this->unknown2ac3e0 = 0;
}
