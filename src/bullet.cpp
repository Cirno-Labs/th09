#include "bullet.h"
#include "enemy.h"
#include "gamemanager.h"

void Bullet::ClearCommands()
{
    int i;

    this->commandIndex = 0;
    this->flags = 0;
    this->unknownDb4 = 0;
    for (i = 0; i < 18; i++)
    {
        this->commands[i].flag = 0;
    }
}

void BulletManager::ClearUnknown25e198()
{
    this->unknown25e198[0] = this->unknown25e198[1] = this->unknown25e198[2] = this->unknown25e198[3] =
        this->unknown25e198[4] = this->unknown25e198[5] = 0;
}

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

void BulletManager::Unknown412590(int arg)
{
}

void BulletManager::Unknown4125a0(int arg, int arg2)
{
}

int BulletManager::LoadAnm()
{
    return g_SpriteEngine->PreloadAnm(8, "etama.anm") == NULL;
}

void BulletManager::ReleaseAnm()
{
    g_SpriteEngine->ReleaseAnm(8);
}

void Bullet::SetAccelCommand(int i, int concurrent, int frames, float arg0, float arg1)
{
    BulletCommand *cmd = this->SetCommand(i, concurrent, 0x10);

    cmd->frames = frames;
    cmd->arg0 = arg0;
    cmd->arg1 = arg1;
}

void Bullet::SetCommand20000(int i, int concurrent, int frames)
{
    BulletCommand *cmd = this->SetCommand(i, concurrent, 0x20000);

    cmd->frames = frames;
}

void Bullet::SetSpriteCommand(int i, int concurrent, int type, int color)
{
    BulletCommand *cmd = this->SetCommand(i, concurrent, 0x4000);

    cmd->frames = type;
    cmd->maxTurns = color;
}

void EnemyShooter::SetAngleAccelCommand(int i, int concurrent, int frames, float arg1, float arg0)
{
    BulletCommand *cmd = this->SetCommand(i, concurrent, 0x20);

    cmd->frames = frames;
    cmd->arg0 = arg0;
    cmd->arg1 = arg1;
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

void EnemyShooter::SetAccelCommand(int i, int concurrent, int frames, float arg0, float arg1)
{
    BulletCommand *cmd = this->SetCommand(i, concurrent, 0x10);

    cmd->frames = frames;
    cmd->arg0 = arg0;
    cmd->arg1 = arg1;
}

void EnemyShooter::SetCommand20000(int i, int concurrent, int frames)
{
    BulletCommand *cmd = this->SetCommand(i, concurrent, 0x20000);

    cmd->frames = frames;
}

void Bullet::FreeBullet()
{
    this->state = 0;
    this->timer = 0;
    this->grazeDelayTimer = 0;
}
