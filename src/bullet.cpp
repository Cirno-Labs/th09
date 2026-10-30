#include "bullet.h"
#include "gamemanager.h"

void BulletManager::Initialize()
{
    int i;
    Bullet *bullet;

    memset(this, 0, sizeof(BulletManager));
    this->nextBullet = this->bullets;
    this->bullets[0xaf].state = 6;
    this->bullets2[0x168].state = 6;
    this->nextBullet2 = this->bullets2;
    this->clearItemType = 5;
    bullet = this->bullets;
    for (i = 0; i < 0xaf; i++, bullet++)
    {
        bullet->sprites.sprite0.scriptIndex = -1;
        bullet->sprites.sprite4.scriptIndex = -1;
        bullet->sprites.sprite1.scriptIndex = -1;
        bullet->sprites.sprite2.scriptIndex = -1;
        bullet->sprites.sprite3.scriptIndex = -1;
    }
    bullet = this->bullets2;
    for (i = 0; i < 0x168; i++, bullet++)
    {
        bullet->sprites.sprite0.scriptIndex = -1;
        bullet->sprites.sprite4.scriptIndex = -1;
        bullet->sprites.sprite1.scriptIndex = -1;
        bullet->sprites.sprite2.scriptIndex = -1;
        bullet->sprites.sprite3.scriptIndex = -1;
    }
}

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

BulletManager::BulletManager()
{
    this->Initialize();
}

BulletManager *BulletManager::RegisterChain(int sideIdx)
{
    BulletManager *mgr =
        (BulletManager *)g_MemoryManager.Register(new BulletManager(), sizeof(BulletManager), "EtamaCtrlInf");

    mgr->Initialize();
    mgr->sideIdx = sideIdx;
    mgr->side = &g_GameManager.sides[sideIdx];
    mgr->otherSide = &g_GameManager.sides[1 - sideIdx];
    mgr->calcElem = g_Chain.CreateElem((ChainCallback)BulletManager::OnUpdate);
    mgr->calcElem->arg = mgr;
    mgr->calcElem->addedCallback = (ChainCallback)BulletManager::AddedCallback;
    if (g_Chain.AddToCalcChain(mgr->calcElem, sideIdx + 0xc) != 0)
    {
        return NULL;
    }
    mgr->drawElem = g_Chain.CreateElem((ChainCallback)BulletManager::OnDraw);
    mgr->drawElem->arg = mgr;
    g_Chain.AddToDrawChain(mgr->drawElem, sideIdx + 0x15);
    return mgr;
}
