#include <string.h>

#include "effect.h"
#include "enemy.h"
#include "gamemanager.h"
#include "sprite.h"
#include "unsorted.h"

void Enemy::ResetRankSpeeds()
{
    this->rankSpeedLow = -0.5f;
    this->rankSpeedHigh = 0.5f;
    this->rankCount1Low = 0;
    this->rankCount1High = 0;
    this->rankCount2Low = 0;
    this->rankCount2High = 0;
}

void Enemy::ReleaseEffects()
{
    int i;

    for (i = 0; i < this->effectCount; i++)
    {
        if (this->effects[i] == NULL)
        {
            continue;
        }
        this->effects[i]->fadeOut = 1;
        this->effects[i] = NULL;
    }
    this->effectCount = 0;
}

float LerpAngleShort(float from, float to, float t)
{
    float diff;
    float wrapped;

    if (from < to)
    {
        diff = to - from;
        wrapped = from + 6.2831855f - to;
    }
    else
    {
        diff = from - to;
        wrapped = to + 6.2831855f - from;
        from = to;
    }
    if (!(diff < wrapped))
    {
        diff = wrapped;
    }
    return diff * t + from;
}

Enemy *EnemyManager::FindByUnknownType(int type)
{
    int i;
    Enemy *enemy = this->enemies;

    for (i = 0; i < 0x80; i++, enemy++)
    {
        if (enemy->unknownType == type && enemy->active)
        {
            return enemy;
        }
    }
    return NULL;
}

void Enemy::Move()
{
    this->posDelta = this->pos - this->prevPos;
    this->prevPos = this->pos;
    if (this->useVelocity2)
    {
        this->pos.x += g_App.gameSpeed * this->velocity2.x;
        this->pos.y += g_App.gameSpeed * this->velocity2.y;
        return;
    }
    if (!this->mirrored)
    {
        this->pos.x += g_App.gameSpeed * this->velocity.x;
    }
    else
    {
        this->pos.x -= g_App.gameSpeed * this->velocity.x;
    }
    this->pos.y += g_App.gameSpeed * this->velocity.y;
}

void EnemyManager::Initialize()
{
    int i;
    Enemy *enemy;

    memset(this, 0, sizeof(EnemyManager));
    for (i = 0; i < ARRAY_SIZE(this->timelineSignals); i++)
    {
        this->timelineSignals[i] = -1;
    }
    enemy = &this->enemyTemplate;
    memset(enemy, 0, sizeof(Enemy));
    enemy->manager = this;
    for (i = 0; i < 2; i++)
    {
        enemy->sprites[i].scriptIndex = -1;
    }
    for (i = 0; i < 0x60; i++)
    {
        enemy->trailBoxes[i].pos.x = -999.0f;
    }
    enemy->active = 1;
    enemy->callbackTimer = 0;
    enemy->onScreen = 0;
    enemy->hitboxSize = D3DXVECTOR3(24.0f, 24.0f, 24.0f);
    enemy->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    enemy->angularVelocity = 0.0f;
    enemy->angle = 0.0f;
    enemy->acceleration = 0.0f;
    enemy->speed = 0.0f;
    enemy->moveMode = 0;
    enemy->holdFire = 0;
    enemy->mirrored = 0;
    enemy->isBoss = 0;
    enemy->stackDepth = 0;
    enemy->life = 1;
    enemy->score = 100;
    enemy->deathEffect = 0;
    enemy->deathEffectIndex = 0;
    enemy->deathEffectVariant = 0;
    enemy->shootInterval = 0;
    enemy->shootTimer = 0;
    enemy->shootOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    enemy->turnLeft = -1;
    enemy->turnRight = -1;
    enemy->turnStill = -1;
    enemy->shootable = 1;
    enemy->contactDamage = 1;
    enemy->hideFollowEffects = 0;
    enemy->hitByShots = 1;
    enemy->grazeable = 0;
    enemy->deathMode = 0;
    enemy->deathSub = -1;
    enemy->clampPos = 0;
    enemy->effectCount = 0;
    enemy->pendingInterrupt = -1;
    for (i = 0; i < 4; i++)
    {
        enemy->lifeCallbackLife[i] = -1;
    }
    enemy->timerCallbackTime = -1;
    enemy->laserIndex = 0;
    enemy->hitFlashTimer = 0;
    enemy->rotateSprite = 0;
    enemy->rankSpeedLow = -0.15f;
    enemy->rankSpeedHigh = 0.15f;
    enemy->shooter.sound = 7;
    enemy->shooter.bulletSound = 0x19;
    enemy->minShootDistSq = 1024.0f;
}

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
