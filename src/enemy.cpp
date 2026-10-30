#include "enemy.h"
#include "sprite.h"

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
