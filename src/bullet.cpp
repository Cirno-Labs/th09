#include "bullet.h"

void BulletManager::Reset()
{
    memset(this->bullets, 0, sizeof(this->bullets));
    memset(this->bullets2, 0, sizeof(this->bullets2));
    memset(this->lasers, 0, sizeof(this->lasers));
    this->nextBullet = this->bullets;
    this->nextBullet2 = this->bullets2;
    this->bullets[0xaf].state = 6;
    this->bullets2[0x168].state = 6;
    this->clearItemType = 5;
}

void BulletManager::RemoveChain(BulletManager *mgr)
{
    if (mgr != NULL)
    {
        g_Chain.Remove(mgr->drawElem);
        g_Chain.Remove(mgr->calcElem);
        delete mgr;
    }
}
