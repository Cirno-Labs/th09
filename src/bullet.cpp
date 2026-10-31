#include "bullet.h"
#include "enemy.h"
#include "gamemanager.h"
#include "player.h"
#include "unsorted.h"

void Bullet::ClearCommands()
{
    int i;

    this->commandIndex = 0;
    this->flags = 0;
    this->activeFlags = 0;
    for (i = 0; i < 18; i++)
    {
        this->commands[i].flag = 0;
    }
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

void PlayerUnknown36c::AddTurned(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, D3DXVECTOR3 *origin, float angle,
                                 void *unknown2c)
{
    if (this->count < 0x80)
    {
        this->entries[this->count].pos = *pos;
        this->entries[this->count].unknownC = *origin;
        this->entries[this->count].size = *size;
        this->entries[this->count].size.x /= 2.0f;
        this->entries[this->count].size.y /= 2.0f;
        this->entries[this->count].radius = 0.0f;
        this->entries[this->count].angle = angle;
        this->entries[this->count].unknown2c = unknown2c;
        this->count++;
    }
}

void Bullet::FreeBullet()
{
    this->state = 0;
    this->timer = 0;
    this->grazeDelayTimer = 0;
}

// Flag 1: the bullet starts 5 faster and slows to its speed over 16
// frames, as th08's (0x413460).
static void SlowBulletStart(Bullet *bullet)
{
    if (bullet->ex[0].timer <= 16)
    {
        SetPolar((Vector2 *)&bullet->velocity, bullet->angle,
                 (5.0f - bullet->ex[0].timer.AsFloat() * 5.0f / 16.0f + bullet->speed) * g_App.gameSpeed);
    }
    else
    {
        bullet->activeFlags ^= 1;
    }
    bullet->ex[0].timer++;
}

// Flag 0x10: adds ex[1].accel to the velocity for ex[1].duration frames,
// as th08's (0x4134d0).
static void AccelerateAlongVelocity(Bullet *bullet)
{
    if (bullet->ex[1].timer >= bullet->ex[1].duration)
    {
        bullet->activeFlags &= ~0x10;
    }
    else
    {
        bullet->velocity += bullet->ex[1].accel * g_App.gameSpeed;
        if (Fabs(bullet->velocity.x) > 0.0001f || Fabs(bullet->velocity.y) > 0.0001f)
        {
            bullet->angle = Atan2(bullet->velocity.y, bullet->velocity.x);
        }
    }
    bullet->ex[1].timer++;
}

// Flag 0x20: turns and speeds up for ex[2].duration frames, as th08's
// (0x413590).
static void TurnAndAccelerate(Bullet *bullet)
{
    if (bullet->ex[2].timer >= bullet->ex[2].duration)
    {
        bullet->activeFlags &= ~0x20;
    }
    else
    {
        bullet->angle = AddNormalizeAngle(bullet->angle, g_App.gameSpeed * bullet->ex[2].turnAngle);
        bullet->speed = g_App.gameSpeed * bullet->ex[2].turnSpeed + bullet->speed;
        SetPolar((Vector2 *)&bullet->velocity, bullet->angle, g_App.gameSpeed * bullet->speed);
    }
    bullet->ex[2].timer++;
}

// Flag 0x40: slows to a stop over ex[3].duration frames, then turns by
// ex[3].turnAngle and starts again, as th08's (0x413630).
static void StopTurnAndRestart(Bullet *bullet)
{
    float speed;
    int duration = bullet->ex[3].duration;

    if (bullet->ex[3].timer >= duration)
    {
        if (bullet->sound >= 0)
        {
            g_SoundPlayer.PlaySoundByIdx(bullet->sound, 0);
        }
        bullet->ex[3].turnCount++;
        if (bullet->ex[3].turnCount >= bullet->ex[3].maxTurns)
        {
            bullet->activeFlags &= ~0x40;
        }
        bullet->angle += bullet->ex[3].turnAngle;
        bullet->speed = bullet->ex[3].turnSpeed;
        speed = bullet->speed;
        bullet->ex[3].timer = 0;
    }
    else
    {
        speed = bullet->speed - bullet->ex[3].timer.AsFloat() * bullet->speed / duration;
    }
    SetPolar((Vector2 *)&bullet->velocity, bullet->angle, g_App.gameSpeed * speed);
    bullet->ex[3].timer++;
}

// Flag 0x100: the same, turning to ex[3].turnAngle, as th08's (0x413700).
static void StopTurnToAngle(Bullet *bullet)
{
    float speed;
    int duration = bullet->ex[3].duration;

    if (bullet->ex[3].timer >= duration)
    {
        if (bullet->sound >= 0)
        {
            g_SoundPlayer.PlaySoundByIdx(bullet->sound, 0);
        }
        bullet->ex[3].turnCount++;
        if (bullet->ex[3].turnCount >= bullet->ex[3].maxTurns)
        {
            bullet->activeFlags &= ~0x100;
        }
        bullet->angle = bullet->ex[3].turnAngle;
        bullet->speed = bullet->ex[3].turnSpeed;
        speed = bullet->speed;
        bullet->ex[3].timer = 0;
    }
    else
    {
        speed = bullet->speed - bullet->ex[3].timer.AsFloat() * bullet->speed / duration;
    }
    SetPolar((Vector2 *)&bullet->velocity, bullet->angle, g_App.gameSpeed * speed);
    bullet->ex[3].timer++;
}

// Flag 0x80: the same, turning to ex[3].turnAngle off the side's player,
// as th08's (0x4137d0).
static void StopTurnToPlayer(Bullet *bullet)
{
    float speed;
    int duration = bullet->ex[3].duration;

    if (bullet->ex[3].timer >= duration)
    {
        if (bullet->sound >= 0)
        {
            g_SoundPlayer.PlaySoundByIdx(bullet->sound, 0);
        }
        bullet->ex[3].turnCount++;
        if (bullet->ex[3].turnCount >= bullet->ex[3].maxTurns)
        {
            bullet->activeFlags &= ~0x80;
        }
        bullet->angle =
            AddNormalizeAngle(bullet->mgr->side->player->AngleToPlayer(&bullet->pos), bullet->ex[3].turnAngle);
        bullet->speed = bullet->ex[3].turnSpeed;
        speed = bullet->speed;
        bullet->ex[3].timer = 0;
    }
    else
    {
        speed = bullet->speed - bullet->ex[3].timer.AsFloat() * bullet->speed / duration;
    }
    SetPolar((Vector2 *)&bullet->velocity, bullet->angle, g_App.gameSpeed * speed);
    bullet->ex[3].timer++;
}

// Flags 0x400 and 0x800: bounces off the sides and the top (and the
// bottom with 0x400) up to ex[4].maxTurns times, as th08's (0x4138c0).
static void BounceOffWalls(Bullet *bullet)
{
    float speed;

    if (!g_GameManager.IsInPlayArea(bullet->pos[0], bullet->pos[1], bullet->sprites.sprite0.spriteData->width,
                                      bullet->sprites.sprite0.spriteData->height))
    {
        if (bullet->sound >= 0)
        {
            g_SoundPlayer.PlaySoundByIdx(bullet->sound, 0);
        }
        if (bullet->pos.x < -144.0f || bullet->pos.x >= 144.0f)
        {
            bullet->angle = -bullet->angle - 3.1415927f;
            bullet->angle = AddNormalizeAngle(bullet->angle, 0.0f);
        }
        if (bullet->pos.y < 0.0f || (bullet->pos.y >= 448.0f && (bullet->activeFlags & 0x400)))
        {
            bullet->angle = -bullet->angle;
        }
        bullet->speed = bullet->ex[4].turnSpeed;
        speed = bullet->speed;
        SetPolar((Vector2 *)&bullet->velocity, bullet->angle, speed * g_App.gameSpeed);
        bullet->ex[4].duration++;
        if (bullet->ex[4].duration >= bullet->ex[4].maxTurns)
        {
            bullet->activeFlags &= ~0xc00;
        }
    }
}

// Flag 0x400000: wraps around from one side to the other for ex[6]'s
// frames, as th08's (0x4139f0).
static void WrapAroundX(Bullet *bullet)
{
    if (bullet->pos.x < 0.0)
    {
        bullet->pos.x += 384.0f;
    }
    else if (bullet->pos.x > 384.0)
    {
        bullet->pos.x -= 384.0f;
    }
    if (bullet->ex[6].timer <= 0)
    {
        bullet->activeFlags ^= 0x400000;
    }
    else
    {
        bullet->ex[6].timer--;
    }
}

// Flag 0x800000: the same from the top to the bottom, as th08's
// (0x413a70).
static void WrapAroundY(Bullet *bullet)
{
    if (bullet->pos.y < 0.0)
    {
        bullet->pos.y += 448.0f;
    }
    else if (bullet->pos.y > 448.0)
    {
        bullet->pos.y -= 448.0f;
    }
    if (bullet->ex[6].timer <= 0)
    {
        bullet->activeFlags ^= 0x800000;
    }
    else
    {
        bullet->ex[6].timer--;
    }
}

int BulletManager::OnUpdate(BulletManager *mgr)
{
    Bullet *bullet;
    int i;
    int j;
    int fadeTime;
    int res;
    Laser *las;
    int alpha;
    float scale;

    bullet = mgr->bullets;
    if (g_GameManager.stateFlags & 0x1800)
    {
        return 1;
    }
    g_App.SetCamera(mgr->sideIdx);
    mgr->bulletCount = 0;
    mgr->bulletCount1 = 0;
    mgr->bulletCount2 = 0;
    mgr->ClearUnknown25e198();
    for (i = 0; i < 0x218; bullet++, i++)
    {
        if (bullet->state == 0 || bullet->state == 6)
        {
            continue;
        }
        if (mgr->side->flag0)
        {
            bullet->next = mgr->unknown25e198[bullet->sprites.sizeClass];
            mgr->unknown25e198[bullet->sprites.sizeClass] = bullet;
            continue;
        }
        mgr->bulletCount++;
        if (i < 0xaf)
        {
            mgr->bulletCount1++;
        }
        else
        {
            mgr->bulletCount2++;
        }
        switch (bullet->state)
        {
        spawned:
            bullet->state = 1;
            bullet->timer = 0;
        case 1:
            RunBulletCommands(bullet);
            if (bullet->activeFlags != 0)
            {
                if (bullet->activeFlags & 1)
                {
                    SlowBulletStart(bullet);
                }
                if (bullet->activeFlags & 0x10)
                {
                    AccelerateAlongVelocity(bullet);
                }
                if (bullet->activeFlags & 0x20)
                {
                    TurnAndAccelerate(bullet);
                }
                if (bullet->activeFlags & 0x40)
                {
                    StopTurnAndRestart(bullet);
                }
                if (bullet->activeFlags & 0x100)
                {
                    StopTurnToAngle(bullet);
                }
                if (bullet->activeFlags & 0x80)
                {
                    StopTurnToPlayer(bullet);
                }
                if (bullet->activeFlags & 0xc00)
                {
                    BounceOffWalls(bullet);
                }
                if (bullet->activeFlags & 0x400000)
                {
                    WrapAroundX(bullet);
                }
                if (bullet->activeFlags & 0x800000)
                {
                    WrapAroundY(bullet);
                }
                DWORD flags = bullet->activeFlags;
                if (flags & 0x20000)
                {
                    if (bullet->ex[5].timer <= 0)
                    {
                        bullet->activeFlags = flags ^ 0x20000;
                    }
                    else
                    {
                        bullet->ex[5].timer--;
                    }
                }
            }
            if (bullet->boundsCheckDelay != 0)
            {
                bullet->boundsCheckDelay--;
            }
            bullet->pos += bullet->velocity;
            if (bullet->boundsCheckDelay == 0)
            {
                if (!g_GameManager.IsInPlayArea(bullet->pos[0], bullet->pos[1],
                                                  bullet->sprites.sprite0.spriteData->width,
                                                  bullet->sprites.sprite0.spriteData->height))
                {
                    if (bullet->activeFlags & 0xdc0)
                    {
                        bullet->outOfBoundsFrames++;
                        if (bullet->outOfBoundsFrames >= 0x80)
                        {
                            bullet->FreeBullet();
                            continue;
                        }
                    }
                    else
                    {
                        if (bullet->outOfBoundsFrames == 0)
                        {
                            bullet->FreeBullet();
                            continue;
                        }
                        bullet->outOfBoundsFrames--;
                    }
                }
                else
                {
                    bullet->outOfBoundsFrames = 0;
                }
            }
            if (!bullet->harmless)
            {
                if (bullet->grazed == 0)
                {
                    res = mgr->side->player->CheckGraze(&bullet->pos, &bullet->sprites.hitboxSize, bullet);
                    if (res == 1)
                    {
                        bullet->grazed = 1;
                        goto checkHit;
                    }
                    if (res == 2 && !(bullet->flags & 0x1000))
                    {
                        bullet->state = 5;
                    }
                }
                else
                {
                checkHit:
                    res = mgr->side->player->CheckHitAreas(&bullet->pos, &bullet->sprites.hitboxSize, bullet);
                    if (res != 0 && (res != 2 || !(bullet->flags & 0x1000)))
                    {
                        bullet->state = 5;
                    }
                    else
                    {
                        mgr->side->player->unknown36c.Add(&bullet->pos, &bullet->sprites.hitboxSize, bullet);
                    }
                }
            }
            if (bullet->sprites.sprite0.scriptPos != NULL)
            {
                g_SpriteEngine->Step(&bullet->sprites.sprite0);
            }
            break;
        case 2:
            bullet->grazeDelayTimer--;
            bullet->pos += bullet->velocity / 2.0f;
            if (g_SpriteEngine->Step(&bullet->sprites.sprite1) == 0)
            {
                break;
            }
            if (bullet->spawnedInArea)
            {
                bullet->state = 5;
            }
            goto spawned;
        case 3:
            bullet->grazeDelayTimer--;
            bullet->pos += bullet->velocity / 2.5f;
            if (g_SpriteEngine->Step(&bullet->sprites.sprite2) == 0)
            {
                break;
            }
            if (bullet->spawnedInArea)
            {
                bullet->state = 5;
            }
            goto spawned;
        case 4:
            bullet->grazeDelayTimer--;
            bullet->pos += bullet->velocity / 3.0f;
            if (g_SpriteEngine->Step(&bullet->sprites.sprite3) == 0)
            {
                break;
            }
            if (bullet->spawnedInArea)
            {
                bullet->state = 5;
            }
            goto spawned;
        case 5:
            bullet->pos += bullet->velocity / 2.0f;
            if (g_SpriteEngine->Step(&bullet->sprites.sprite4) != 0)
            {
                bullet->FreeBullet();
                continue;
            }
        }
        bullet->timer++;
        bullet->grazeDelayTimer++;
        bullet->next = mgr->unknown25e198[bullet->sprites.sizeClass];
        mgr->unknown25e198[bullet->sprites.sizeClass] = bullet;
    }
    if (!mgr->side->flag0)
    {
        las = mgr->lasers;
        D3DXVECTOR3 center;
        D3DXVECTOR3 extent;
        for (j = 0x30; j != 0; j--, las++)
        {
            if (!las->active)
            {
                continue;
            }
            las->endOffset = g_App.gameSpeed * las->speed + las->endOffset;
            if (las->endOffset - las->startOffset > las->maxLength)
            {
                las->startOffset = las->endOffset - las->maxLength;
            }
            if (las->startOffset < 0.0f)
            {
                las->startOffset = 0.0f;
            }
            extent.y = las->width / 2.0f;
            center.y = las->pos.y;
            scale = las->endOffset - las->startOffset;
            extent.x = scale;
            center.x = scale / 2.0f + las->startOffset + las->pos.x;
            las->sprite0.scaleY = las->width / las->sprite0.spriteData->width;
            las->sprite0.scaleX = scale / las->sprite0.spriteData->height;
            las->sprite0.SetRotationZ(AddNormalizeAngle(las->angle + 1.5707964f, 0.0f));
            switch (las->state)
            {
            case 0:
                if (las->flags & 1)
                {
                    alpha = las->timer.AsFloat() * 255.0f / las->startFrames;
                    if (alpha > 0xff)
                    {
                        alpha = 0xff;
                    }
                    las->sprite0.color = alpha << 24;
                }
                else
                {
                    int frames = las->startFrames;

                    fadeTime = frames > 30 ? 30 : frames;
                    if (frames - fadeTime < las->timer.AsFrames())
                    {
                        scale = las->timer.AsFloat() * las->width / frames;
                    }
                    else
                    {
                        scale = 1.2f;
                    }
                    las->baseWidth = scale;
                    las->sprite0.scaleY = scale / 16.0f;
                    extent.x = scale / 2.0f;
                }
                if (las->timer >= las->hitStartFrame)
                {
                    mgr->side->player->unknown36c.AddTurned(&center, &extent, &las->pos, las->angle, NULL);
                }
                if (las->timer < las->startFrames)
                {
                    break;
                }
                las->timer = 0;
                las->state++;
                las->baseWidth = las->width;
            case 1:
                mgr->side->player->unknown36c.AddTurned(&center, &extent, &las->pos, las->angle, NULL);
                if (las->timer < las->durationFrames)
                {
                    break;
                }
                las->timer = 0;
                las->state++;
                if (las->endFrames == 0)
                {
                    las->active = 0;
                    continue;
                }
            case 2:
                if (las->flags & 1)
                {
                    alpha = las->timer.AsFloat() * 255.0f / las->startFrames;
                    if (alpha > 0xff)
                    {
                        alpha = 0xff;
                    }
                    las->sprite0.color = alpha << 24;
                }
                else
                {
                    int frames = las->endFrames;

                    if (frames > 0)
                    {
                        scale = las->width - las->timer.AsFloat() * las->width / frames;
                        las->sprite0.scaleY = scale / 16.0f;
                        extent.x = scale / 2.0f;
                    }
                }
                if (las->timer < las->endHitFrames)
                {
                    mgr->side->player->unknown36c.AddTurned(&center, &extent, &las->pos, las->angle, NULL);
                }
                if (las->timer < las->endFrames)
                {
                    break;
                }
                las->active = 0;
                continue;
            }
            if (las->startOffset >= 640.0f)
            {
                las->active = 0;
            }
            las->timer++;
            g_SpriteEngine->Step(&las->sprite0);
        }
        if (mgr->clearAllFrames != 0)
        {
            mgr->clearAllFrames--;
        }
        mgr->updateTimer++;
        mgr->updateFrameCount++;
    }
    return 1;
}
