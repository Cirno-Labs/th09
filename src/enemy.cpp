#include <string.h>

#include "ascii.h"
#include "effect.h"
#include "enemy.h"
#include "gamemanager.h"
#include "gui.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// Timer::operator== is the original's ordinary function of this object
// (at 0x404950, before the ECL code): EnemyTimeline::Run relies on it
// keeping esi and the timeline's fields across the call.
BOOL Timer::operator==(int frames)
{
    return this->current == frames;
}

void Enemy::ResetRankSpeeds()
{
    this->rankSpeedLow = -0.5f;
    this->rankSpeedHigh = 0.5f;
    this->rankCount1Low = 0;
    this->rankCount1High = 0;
    this->rankCount2Low = 0;
    this->rankCount2High = 0;
}

Enemy *EnemyManager::SpawnEnemy(int sub, D3DXVECTOR3 *pos, int life, int itemDrop, int score, int mirrored,
                                int usePlayerEcl)
{
    Enemy *enemy;
    int i;

    enemy = this->enemies;
    for (i = 0; i < 0x80; i++, enemy++)
    {
        if (enemy->active)
        {
            continue;
        }
        *enemy = this->enemyTemplate;
        enemy->index = i;
        enemy->mirrored = mirrored;
        if (life >= 0)
        {
            enemy->life = life;
        }
        enemy->pos = *pos;
        if (mirrored)
        {
            enemy->pos.x = -enemy->pos.x;
        }
        if (!usePlayerEcl)
        {
            this->ecl.StartSub(&enemy->unknown7f4, sub);
            if (this->ecl.RunEnemy(enemy) == -1)
            {
                enemy->active = 0;
                i = 0x80;
                break;
            }
        }
        else
        {
            this->playerEcl.StartSub(&enemy->unknown7f4, sub);
            if (this->playerEcl.RunEnemy(enemy) == -1)
            {
                enemy->active = 0;
                i = 0x80;
                break;
            }
        }
        enemy->color = enemy->primarySprite.color;
        enemy->item = (char)itemDrop;
        if (score >= 0)
        {
            enemy->score = score;
        }
        enemy->maxLife = enemy->life;
        enemy->phaseStartLife = enemy->life;
        break;
    }
    this->spawnFailed = i == 0x80;
    return enemy;
}

Enemy *EnemyManager::SpawnEnemyWithVars(int sub, D3DXVECTOR3 *pos, int life, char itemDrop, int score,
                                        EclVars *vars, int usePlayerEcl)
{
    Enemy *enemy;
    int i;

    enemy = this->enemies;
    for (i = 0; i < 0x80; i++, enemy++)
    {
        if (enemy->active)
        {
            continue;
        }
        *enemy = this->enemyTemplate;
        enemy->index = i;
        if (life >= 0)
        {
            enemy->life = life;
        }
        enemy->pos = *pos;
        if (!usePlayerEcl)
        {
            this->ecl.StartSub(&enemy->unknown7f4, sub);
            enemy->unknown7f4.vars = *vars;
            if (this->ecl.RunEnemy(enemy) == -1)
            {
                enemy->active = 0;
                i = 0x80;
                break;
            }
        }
        else
        {
            this->playerEcl.StartSub(&enemy->unknown7f4, sub);
            enemy->unknown7f4.vars = *vars;
            if (this->playerEcl.RunEnemy(enemy) == -1)
            {
                enemy->active = 0;
                i = 0x80;
                break;
            }
        }
        enemy->color = enemy->primarySprite.color;
        enemy->item = itemDrop;
        if (life >= 0)
        {
            enemy->life = life;
        }
        if (score >= 0)
        {
            enemy->score = score;
        }
        enemy->maxLife = enemy->life;
        enemy->phaseStartLife = enemy->life;
        break;
    }
    this->spawnFailed = i == 0x80;
    return enemy;
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

int EnemyManager::ClearEnemiesForBonus(int maxBonus, int start)
{
    Enemy *enemy = this->enemies;
    int total = start;
    int bonus = 2000;
    int k;
    int j;

    for (k = 0; k < 0x80; k++, enemy++)
    {
        if (!enemy->active)
        {
            continue;
        }
        if (enemy->isBoss)
        {
            continue;
        }
        if (enemy->undying)
        {
            continue;
        }
        enemy->life = 0;
        if (enemy->grazeable)
        {
            enemy->absPos = enemy->pos + enemy->posOrigin;
            g_AsciiText.AddPopup(this->sideIdx, &enemy->absPos, bonus, bonus >= maxBonus ? 0xffffff00 : 0xffffffff);
            total += bonus;
            bonus += 30;
            if (bonus > maxBonus)
            {
                bonus = maxBonus;
            }
            if (enemy->trailFlags)
            {
                for (j = 0; j < enemy->trailSegmentCount; j += 6)
                {
                    g_AsciiText.AddPopup(this->sideIdx, &enemy->trailBoxes[j].pos, bonus,
                                         bonus >= maxBonus ? 0xffffff00 : 0xffffffff);
                    total += bonus;
                    bonus += 30;
                    if (bonus > maxBonus)
                    {
                        bonus = maxBonus;
                    }
                }
            }
        }
        if (enemy->deathSub >= 0)
        {
            this->ecl.StartSub(&enemy->unknown7f4, enemy->deathSub);
            enemy->deathSub = -1;
        }
    }
    return total;
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

int EnemyTimeline::Run()
{
    if (g_GameManager.gui->IsMsgRunning())
    {
        return 0;
    }
    if (this->delay.AsFloat() > 0.0f)
    {
        this->delay--;
        return 0;
    }
    while (this->pos->time >= 0)
    {
        if (this->timer == this->pos->time)
        {
            if (!(this->pos->difficulties & g_GameManager.difficultyMask))
            {
                goto next;
            }
            switch (this->pos->opcode)
            {
            case 16:
                g_GameManager.isInContinueMenu = 1;
                break;
            case 0:
            case 1:
            {
                EnemyTimelineSpawn *args = (EnemyTimelineSpawn *)this->pos->args;
                D3DXVECTOR3 pos;

                pos.x = args->x;
                pos.y = args->y;
                pos.z = 0.0f;
                this->mgr->SpawnEnemy(args->sub, &pos, args->life, args->item, args->score, this->mirror, 0);
            }
                break;
            case 17:
            {
                EnemyTimelineSpawn *args = (EnemyTimelineSpawn *)this->pos->args;
                D3DXVECTOR3 pos;
                Enemy *enemy;

                pos.x = args->x;
                pos.y = args->y;
                pos.z = 0.0f;
                enemy = this->mgr->SpawnEnemy(args->sub, &pos, args->life, args->item, args->score, this->mirror, 0);
                enemy->unknownBit9 = 1;
            }
                break;
            case 15:
            {
                EnemyTimelineSpawn *args = (EnemyTimelineSpawn *)this->pos->args;
                D3DXVECTOR3 pos;

                pos.x = args->x;
                pos.y = args->y;
                pos.z = 0.0f;
                this->mgr->SpawnEnemy(args->sub, &pos, args->life, args->item, args->score, this->mirror, 0);
            }
                break;
            case 12:
                this->mirror = 1;
            case 11:
            {
                EnemyTimelineSpawnWithArgs *args = (EnemyTimelineSpawnWithArgs *)this->pos->args;
                D3DXVECTOR3 pos;
                Enemy *enemy;

                pos.x = args->x;
                pos.y = args->y;
                pos.z = 0.0f;
                enemy = this->mgr->SpawnEnemy(args->sub, &pos, args->life, -1, args->score, this->mirror, 0);
                enemy->pointItems = args->pointItems;
                enemy->powerItems = args->powerItems;
            }
                break;
            case 4:
                this->mirror = 1;
            case 2:
            {
                EnemyTimelineSpawnRandomX *args = (EnemyTimelineSpawnRandomX *)this->pos->args;
                D3DXVECTOR3 pos;

                pos.x = g_Rng.GetRandomF32InRange(args->xMax - args->xMin) + args->xMin;
                pos.y = args->y;
                pos.z = 0.0f;
                this->mgr->SpawnEnemy(args->sub, &pos, args->life, args->item, args->score, this->mirror, 0);
            }
                break;
            case 5:
                this->mirror = 1;
            case 3:
            {
                EnemyTimelineSpawnAnyX *args = (EnemyTimelineSpawnAnyX *)this->pos->args;
                D3DXVECTOR3 pos;

                pos.x = g_Rng.GetRandomF32InRange(384.0f);
                pos.y = args->y;
                pos.z = 0.0f;
                this->mgr->SpawnEnemy(args->sub, &pos, args->life, args->item, args->score, this->mirror, 0);
            }
                break;
            case 8:
                this->mgr->bosses[this->pos->args[0]]->pendingInterrupt = this->pos->args[1];
                break;
            case 10:
                if (this->mgr->bosses[this->pos->args[0]] != NULL && this->mgr->bosses[this->pos->args[0]]->active)
                {
                    this->timer--;
                    goto tick;
                }
                break;
            case 13:
            {
                int count;
                int j;

                count = 0;
                for (j = 0; j < ARRAY_SIZE(this->mgr->timelineSignals); j++)
                {
                    if (this->mgr->timelineSignals[j] == this->pos->args[0])
                    {
                        count++;
                        this->mgr->timelineSignals[j] = -1;
                    }
                }
                if (count == 0)
                {
                    this->timer--;
                    goto tick;
                }
                break;
            }
            case 14:
            {
                int k;

                for (k = 0; k < ARRAY_SIZE(this->mgr->timelineSignals); k++)
                {
                    if (this->mgr->timelineSignals[k] >= 0)
                    {
                        continue;
                    }
                    this->mgr->timelineSignals[k] = this->pos->args[0];
                }
                break;
            }
            }
        }
        else if (this->timer < this->pos->time)
        {
            break;
        }
    next:
        this->pos = (EnemyTimelineEntry *)((char *)this->pos + this->pos->size);
    }
tick:
    if (this->pos->time < 0)
    {
        this->pos = NULL;
        this->timer = 0;
        return 1;
    }
    this->timer += g_GameManager.timelineSpeed;
    return 0;
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
void Enemy::Despawn()
{
    int i;

    if (this->deathMode == 0)
    {
        this->active = 0;
    }
    if (this->isBoss && this->bossIndex < 4)
    {
        this->manager->bosses[this->bossIndex] = NULL;
        this->isBoss = 0;
    }
    if (this->effectCount != 0)
    {
        this->ReleaseEffects();
    }
    if (this->isBoss)
    {
        this->manager->bosses[this->bossIndex] = NULL;
    }
    for (i = 0; i < 4; i++)
    {
        this->lifeCallbackLife[i] = -1;
    }
    this->timerCallbackTime = -1;
    this->FreeUnknown33d8();
    if (this->manager->side->player->unknown3037c == this)
    {
        this->manager->side->player->unknown3037c = NULL;
    }
}

void Enemy::ClampPos()
{
    float *pos;

    if (this->clampPos)
    {
        pos = this->pos;
        if (pos[0] < this->lowerBound[0])
        {
            pos[0] = this->lowerBound[0];
        }
        else if (pos[0] > this->upperBound[0])
        {
            pos[0] = this->upperBound[0];
        }
        if (pos[1] < this->lowerBound[1])
        {
            pos[1] = this->lowerBound[1];
        }
        else if (pos[1] > this->upperBound[1])
        {
            pos[1] = this->upperBound[1];
        }
    }
}

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
    this->timeline.pos = (EnemyTimelineEntry *)this->ecl.GetTimeline(g_TimelineOrder[0] & 0x7f);
    this->timeline.delay = 200;
    this->timeline.timer = 0;
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
