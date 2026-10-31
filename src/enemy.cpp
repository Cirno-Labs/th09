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
            this->ecl.StartSub(&enemy->ctx, sub);
            if (this->ecl.RunEnemy(enemy) == -1)
            {
                enemy->active = 0;
                i = 0x80;
                break;
            }
        }
        else
        {
            this->playerEcl.StartSub(&enemy->ctx, sub);
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
            this->ecl.StartSub(&enemy->ctx, sub);
            enemy->ctx.vars = *vars;
            if (this->ecl.RunEnemy(enemy) == -1)
            {
                enemy->active = 0;
                i = 0x80;
                break;
            }
        }
        else
        {
            this->playerEcl.StartSub(&enemy->ctx, sub);
            enemy->ctx.vars = *vars;
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

// Moves the effects that follow the enemy with it, out to effectRadius
// and round, as th08's.
static void MoveEnemyEffects(Enemy *enemy)
{
    Effect *eff;
    int i;

    for (i = 0; i < enemy->effectCount; i++)
    {
        eff = enemy->effects[i];
        if (eff == NULL)
        {
            continue;
        }
        eff->unknown48 = enemy->pos;
        if (eff->unknown7c < enemy->effectRadius)
        {
            eff->unknown7c += 0.3f;
        }
        else
        {
            eff->unknown7c = enemy->effectRadius;
        }
        eff->unknown80 = AddNormalizeAngle(eff->unknown80, 0.031415928f);
    }
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
            this->ecl.StartSub(&enemy->ctx, enemy->deathSub);
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
        if (this->threads[i] != NULL)
        {
            g_MemoryManager.Free(this->threads[i]);
            this->threads[i] = NULL;
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

int EnemyManager::OnUpdate(EnemyManager *mgr)
{
    int bomb = 0;
    int i;
    D3DXVECTOR3 boxSize;
    int hurt;
    int hit;
    int seg;
    int kind;
    Enemy *enemy;
    Player *player;
    Enemy **head;

    if (g_GameManager.unknownBits134_11)
    {
        return 1;
    }
    g_GameManager.gui->UpdateEnemyMarker(mgr->sideIdx, &D3DXVECTOR3(-999.0f, 0.0f, 0.0f));
    g_App.SetCamera(mgr->sideIdx);
    mgr->drawListHeads[3] = NULL;
    mgr->drawListHeads[2] = NULL;
    mgr->drawListHeads[1] = NULL;
    mgr->drawListHeads[0] = NULL;
    if (!g_GameManager.gui->unknown1095c)
    {
        if (mgr->timeline.pos != NULL)
        {
            mgr->timeline.Run();
        }
        else if (!mgr->unknown2ac3b0 || mgr->unknown2ac3c8 > 0)
        {
            mgr->timeline.mirror = g_TimelineOrder[mgr->unknown2ac3d4] >> 7;
            mgr->timeline.pos =
                (EnemyTimelineEntry *)mgr->ecl.GetTimeline(g_TimelineOrder[mgr->unknown2ac3d4++] & 0x7f);
            mgr->timeline.timer = 0;
        }
    }
    mgr->enemyCount = 0;
    mgr->unknown2ac3b0 = 0;
    mgr->unknown2ac444 = NULL;
    mgr->unknown2ac448 = NULL;
    mgr->unknown2ac3b4 = 0;
    mgr->unknown2ac3b8 = 0;
    enemy = mgr->enemies;
    if (mgr->side->player->unknown364 != NULL)
    {
        mgr->unknown2ac3c8 = 0;
    }
    else
    {
        mgr->unknown2ac3c8++;
    }
    for (i = 0; i < 0x80; i++, enemy++)
    {
        if (!enemy->active)
        {
            if (mgr->side->player->unknown3037c == enemy)
            {
                mgr->side->player->unknown3037c = NULL;
            }
            continue;
        }
        if (mgr->side->flag0 && enemy->unknownType != 1 && enemy->unknownType != 2)
        {
            if (enemy->hideFollowEffects)
            {
                continue;
            }
            head = &mgr->drawListHeads[enemy->drawListIndex];
            enemy->nextInDrawList = *head;
            *head = enemy;
            continue;
        }
        hit = 0;
        if (enemy->unknownFlag8)
        {
            enemy->absPos = enemy->pos + enemy->posOrigin;
            enemy->absPos.z = 0.0f;
            goto die;
        }
        mgr->enemyCount++;
        if (!(enemy->flags2 & 0x4dc0))
        {
            mgr->unknown2ac3b0++;
        }
        if (enemy->unknownType)
        {
            mgr->unknown2ac3b4++;
        }
        if (enemy->unknown3380)
        {
            mgr->unknown2ac3b8++;
        }
        if (enemy->useVelocity2)
        {
            if (enemy->unknown5424.HasTickedEvery(2))
            {
                mgr->side->effectManager->SpawnParticles(9, &enemy->pos, 1, 0xffffffff);
            }
            if (enemy->unknown5424 > mgr->unknown2ac44c)
            {
                EnemyShooter shooter;

                shooter.pos = enemy->pos;
                shooter.sprite = 1;
                shooter.aimMode = 0;
                shooter.speed1 = g_GameManager.rank.unknownB4 * 0.1f + 2.0f;
                shooter.count2 = 1;
                shooter.angle1 = 0.0f;
                shooter.angle2 = 0.15707964f;
                shooter.speed2 = 0.0f;
                shooter.unknown1fa = 0;
                shooter.flags = 4;
                shooter.count1 = (mgr->side->character != 13) * 2 + 1;
                shooter.color = 2;
                mgr->side->bulletManager->FirePattern(&shooter);
                enemy->active = 0;
                enemy->Despawn();
                continue;
            }
        }
        if ((enemy->haltedByBomb && mgr->side->player->GetUnknown0()) || enemy->halted)
        {
            enemy->callbackTimer--;
            goto flash;
        }
    again:
        if (mgr->ecl.RunEnemy(enemy) == -1)
        {
            enemy->active = 0;
            enemy->Despawn();
            continue;
        }
        if (!enemy->isWarpingToBoss)
        {
            enemy->ClampPos();
            enemy->Move();
            enemy->ClampPos();
            enemy->absPos = enemy->pos + enemy->posOrigin;
            enemy->absPos.z = 0.0f;
        }
        else
        {
            enemy->absPos = enemy->pos + enemy->posOrigin;
            enemy->absPos.z = 0.0f;
        }
        if (enemy->trailFlags)
        {
            for (seg = enemy->trailSegmentCount - 1; seg > 0; seg--)
            {
                enemy->trailBoxes[seg].pos = enemy->trailBoxes[seg - 1].pos;
                enemy->trailBoxes[seg].velocity = enemy->trailBoxes[seg - 1].velocity;
                enemy->trailBoxes[seg].angle = enemy->trailBoxes[seg - 1].angle;
            }
            enemy->trailBoxes[0].pos = enemy->absPos;
            enemy->trailBoxes[0].velocity = enemy->velocity;
            enemy->trailBoxes[0].angle = enemy->angle;
        }
        if (enemy->primarySprite.spriteData == NULL)
        {
            enemy->hideFollowEffects = 1;
        }
        if (!enemy->hideFollowEffects && !enemy->onScreen &&
            g_GameManager.IsInPlayArea(enemy->absPos[0], enemy->absPos[1], enemy->primarySprite.spriteData->width,
                                       enemy->primarySprite.spriteData->height))
        {
            enemy->onScreen = 1;
        }
        else if (enemy->onScreen == 1 && !enemy->staysOffscreen &&
                 ((!enemy->trailFlags &&
                   !g_GameManager.IsInPlayArea(enemy->absPos.x, enemy->absPos.y,
                                               enemy->primarySprite.spriteData->width,
                                               enemy->primarySprite.spriteData->height)) ||
                  (enemy->trailFlags &&
                   !g_GameManager.IsInPlayArea(enemy->absPos.x, enemy->absPos.y,
                                               enemy->primarySprite.spriteData->width,
                                               enemy->primarySprite.spriteData->height) &&
                   !g_GameManager.IsInPlayArea(enemy->trailBoxes[enemy->trailSegmentCount - 1].pos.x,
                                               enemy->trailBoxes[enemy->trailSegmentCount - 1].pos.y,
                                               enemy->primarySprite.spriteData->width,
                                               enemy->primarySprite.spriteData->height))))
        {
            enemy->active = 0;
            enemy->Despawn();
            continue;
        }
        if (enemy->CheckLifeCallback())
        {
            goto again;
        }
        if (enemy->timerCallbackTime >= 0 && Enemy::CheckTimerCallback())
        {
            goto again;
        }
        enemy->primarySprite.color = enemy->color;
        g_SpriteEngine->Step(&enemy->primarySprite);
        enemy->color = enemy->primarySprite.color;
        for (seg = 0; seg < 2; seg++)
        {
            if (enemy->sprites[seg].scriptIndex >= 0 && g_SpriteEngine->Step(&enemy->sprites[seg]))
            {
                enemy->sprites[seg].scriptIndex = -1;
            }
        }
        kind = 0;
        if (!enemy->hideFollowEffects && !enemy->unusedFlag5)
        {
            if (enemy->shootable && (!enemy->unknown3380 || mgr->side->character != 11))
            {
                enemy->CheckPlayerContact(&enemy->absPos, &enemy->hitboxSize);
                if (enemy->trailFlags)
                {
                    boxSize = enemy->hitboxSize;
                    for (seg = 1; seg < enemy->trailContactSegments; seg += 6)
                    {
                        if (enemy->trailFlags & 2)
                        {
                            boxSize = enemy->hitboxSize -
                                      enemy->hitboxSize * (float)seg / (float)enemy->trailContactSegments;
                        }
                        enemy->CheckPlayerContact(&enemy->trailBoxes[seg].pos, &boxSize);
                    }
                }
            }
            enemy->lastDamage = 0;
            if (enemy->hitByShots)
            {
                hurt = mgr->side->player->CalcDamage(&enemy->absPos, &enemy->hitboxSize, &enemy->unknown2e5c,
                                                     &bomb, &enemy->unknown2e60);
                if (enemy->extraHitboxSize.x > 0.0f)
                {
                    mgr->side->player->CalcDamage(&enemy->absPos, &enemy->extraHitboxSize, &enemy->unknown2e5c,
                                                  &bomb, &enemy->unknown2e60);
                }
                if (hurt > 0)
                {
                    if (enemy->contactDamage)
                    {
                        if (enemy->unknown53a8 > 0)
                        {
                            if (enemy->isBoss)
                            {
                                hurt /= 9;
                            }
                            else
                            {
                                hurt = 0;
                            }
                        }
                        if (hurt == enemy->unknown2e5c)
                        {
                            kind = 1;
                        }
                        if (hurt == enemy->unknown2e60)
                        {
                            kind = 2;
                        }
                        if (enemy->unknown3380)
                        {
                            if (!enemy->useVelocity2)
                            {
                                hurt = (hurt - enemy->unknown2e5c) * 4 + enemy->unknown2e5c / 4;
                            }
                            else
                            {
                                hurt = (hurt - enemy->unknown2e5c) * 4 + enemy->unknown2e5c / 2;
                            }
                        }
                        enemy->life -= hurt;
                        enemy->lastDamage = hurt;
                    }
                    hit = 1;
                }
                D3DXVECTOR3 targetDiff = mgr->side->player->unknown30364 - mgr->side->player->pos;
                D3DXVECTOR3 toward = enemy->absPos - mgr->side->player->pos;
                if (targetDiff.x * targetDiff.x + targetDiff.y * targetDiff.y > toward.x * toward.x + toward.y * toward.y)
                {
                    mgr->side->player->unknown30364 = enemy->absPos;
                }
                player = mgr->side->player;
                if (Fabs(enemy->absPos.x - player->pos.x) < 64.0f &&
                    (player->unknown3037c == NULL || player->unknown3037c->pos.y > enemy->absPos.y))
                {
                    player->unknown3037c = enemy;
                }
            }
        }
        if (enemy->hasDied && enemy->life > 0)
        {
            enemy->hasDied = 0;
        }
        if (enemy->life <= 0 && !enemy->hasDied && !enemy->undying)
        {
            int k;

        die:
            enemy->hasDied = 1;
            enemy->phaseEndSecondsLeft = (enemy->timerCallbackTime - enemy->callbackTimer.AsFrames()) / 60;
            enemy->timerCallbackTime = -1;
            for (k = 0; k < 4; k++)
            {
                enemy->lifeCallbackLife[k] = -1;
            }
            for (k = 0; k < 4; k++)
            {
                if (enemy->threads[k] != NULL)
                {
                    g_MemoryManager.Free(enemy->threads[k]);
                    enemy->threads[k] = NULL;
                }
            }
            switch (enemy->deathMode)
            {
            case 3:
                enemy->contactDamage = 0;
                enemy->deathMode = 0;
                enemy->life = 1;
                if (enemy->deathEffect >= 0)
                {
                    mgr->side->effectManager->SpawnParticles(enemy->deathEffect + 20, &enemy->absPos, 1, 0xffffffff);
                    mgr->side->effectManager->SpawnParticles(enemy->deathEffect + 20, &enemy->absPos, 1, 0xffffffff);
                    mgr->side->effectManager->SpawnParticles(enemy->deathEffect + 20, &enemy->absPos, 1, 0xffffffff);
                }
                if (mgr->side->player->GetUnknown0() == 0)
                {
                    mgr->side->player->unknown303c8 = 90;
                    mgr->side->player->SetUnknown0(3);
                }
                enemy->haltedByBomb = 0;
                enemy->intangibleDuringBomb = 0;
                break;
            case 1:
                mgr->side->AddScore(enemy->score);
                enemy->unusedFlag20 = 1;
                enemy->shootable = 0;
                enemy->contactDamage = 0;
                enemy->hitByShots = 0;
                goto dead;
            case 0:
                mgr->side->AddScore(enemy->score);
                enemy->active = 0;
            dead:
                if (enemy->isBoss)
                {
                    enemy->ReleaseEffects();
                }
            case 2:
                enemy->life = 0;
                break;
            }
            if (!enemy->unknownFlag8)
            {
                enemy->DropItems(kind);
            }
            if (enemy->deathSub >= 0)
            {
                int l;

                enemy->ResetRankSpeeds();
                enemy->stackDepth = 0;
                for (l = 0; l < 4; l++)
                {
                    enemy->lifeCallbackLife[l] = -1;
                }
                enemy->timerCallbackTime = -1;
                enemy->FreeUnknown33d8();
                enemy->shooter = enemy->manager->enemyTemplate.shooter;
                enemy->shootInterval = 0;
                mgr->ecl.StartSub(&enemy->ctx, enemy->deathSub);
                enemy->deathSub = -1;
            }
        }
    flash:
        if (enemy->hitFlashTimer)
        {
            enemy->hitFlashTimer--;
            enemy->primarySprite.useColor2 = 0;
        }
        else if (hit)
        {
            if (enemy->lifeCallbackProximity < 2)
            {
                g_SoundPlayer.QueueSoundPanned(0x14, enemy->absPos.x);
            }
            else
            {
                g_SoundPlayer.QueueSoundPanned(0x25, enemy->absPos.x);
            }
            enemy->primarySprite.color2Rgb[2] = 0xff;
            enemy->primarySprite.color2Rgb[1] = 0x60;
            enemy->primarySprite.color2Rgb[0] = 0x80;
            enemy->primarySprite.color2Alpha = enemy->primarySprite.alpha;
            enemy->primarySprite.useColor2 = 1;
            enemy->hitFlashTimer = 1;
        }
        else
        {
            enemy->primarySprite.useColor2 = 0;
        }
        if (enemy->isBoss)
        {
            D3DXVECTOR3 markerPos;

            if (enemy->isBoss < 4)
            {
                if (!enemy->hideFollowEffects)
                {
                    markerPos.x = enemy->absPos.x + 32.0f;
                }
                else
                {
                    markerPos.x = -999.0f;
                }
                markerPos.y = 472.0f;
                markerPos.z = 0.0f;
                g_AsciiText.SetUnknown17c4Pos(enemy->bossIndex, &markerPos);
                if (enemy->lifeCallbackProximity == 0)
                {
                    g_AsciiText.SetUnknown2254(enemy->bossIndex, enemy->primarySprite.useColor2);
                }
                else
                {
                    g_AsciiText.SetUnknown2254(enemy->bossIndex, enemy->lifeCallbackProximity + 1);
                }
            }
        }
        if ((enemy->unknownBit9 && mgr->unknown2ac444 == NULL) || enemy->useVelocity2)
        {
            mgr->unknown2ac444 = enemy;
        }
        if (mgr->unknown2ac448 == NULL)
        {
            mgr->unknown2ac448 = enemy;
        }
        if (enemy->unknownType == 3)
        {
            g_GameManager.gui->UpdateEnemyMarker(mgr->sideIdx, &enemy->pos);
            mgr->unknown2ac444 = enemy;
        }
        else if (enemy->unknownBit13)
        {
            mgr->unknown2ac444 = enemy;
        }
        MoveEnemyEffects(enemy);
        enemy->callbackTimer++;
        if (enemy->unknown53a8 > 0)
        {
            enemy->unknown53a8--;
        }
        if (!enemy->hideFollowEffects && enemy->active)
        {
            head = &mgr->drawListHeads[enemy->drawListIndex];
            enemy->nextInDrawList = *head;
            *head = enemy;
        }
    }
    mgr->unknown2ac404++;
    return 1;
}

int EnemyManager::AddedCallback(EnemyManager *mgr)
{
    int i;
    SpriteEngine *se = g_SpriteEngine;
    AnmLoaded *anm = se->GetAnm(9);

    if (anm->file == NULL)
    {
        if (g_GameManager.stage == 0 || g_GameManager.stage == 3 || g_GameManager.stage == 7)
        {
            mgr->anm = se->PreloadAnm(9, "enemy1.anm");
            if (mgr->anm == NULL)
            {
                return -1;
            }
        }
        else if (g_GameManager.stage == 12 || g_GameManager.stage == 15)
        {
            mgr->anm = se->PreloadAnm(9, "enemy13.anm");
            if (mgr->anm == NULL)
            {
                return -1;
            }
        }
        else
        {
            mgr->anm = se->PreloadAnm(9, "enemy.anm");
            if (mgr->anm == NULL)
            {
                return -1;
            }
        }
    }
    else
    {
        mgr->anm = anm;
    }
    mgr->otherPlayerAnm = g_SpriteEngine->GetAnm(6 - mgr->sideIdx);
    memset(&mgr->ecl, 0, sizeof(EclManager));
    if (mgr->ecl.Load("enemy.ecl") != 0)
    {
        return -1;
    }
    if (mgr->playerEcl.Load(g_CharacterFiles[mgr->otherSide->character][2]) != 0)
    {
        return -1;
    }
    if (mgr->sideIdx == 0)
    {
        for (i = 0; i < 0x100; i++)
        {
            g_TimelineOrder[i] = g_Rng.GetRandomU32InRange(mgr->ecl.GetTimelineCount());
            g_TimelineOrder[i] |= g_Rng.GetRandomU32InRange(2) << 7;
        }
    }
    mgr->timeline.mgr = mgr;
    mgr->timeline.pos = (EnemyTimelineEntry *)mgr->ecl.GetTimeline(g_TimelineOrder[mgr->unknown2ac3d4++] & 0x7f);
    mgr->timeline.delay = 200;
    mgr->unknown2ac44c = mgr->side->character == 5 ? 200 : 160;
    return 0;
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
