#include "player.h"
#include "sprite.h"

void PlayerHitArea::Deactivate()
{
    this->active = 0;
}

void PlayerHitArea::Reset()
{
    memset(this, 0, sizeof(PlayerHitArea));
    this->unknown34 = 1;
}

void PlayerUnknown24::Reset()
{
    this->timer5c = 0;
    this->timer68 = 0;
    this->flags &= ~3;
}

void Player::SetRegisterChainArg(BYTE arg)
{
    this->registerChainArg = arg;
}

int Player::OnDraw2(Player *player)
{
    g_App.SetView(player->sideIdx);
    player->DrawHitBullets();
    return 1;
}

int Player::LoadAnms()
{
    int i;
    SpriteEngine *se = g_SpriteEngine;

    for (i = 0; i < 2; i++)
    {
        if (se->GetAnmFile(i + 5) != NULL &&
            g_GameManager.sides[i].character == g_GameManager.sides[i].unknown24)
        {
            continue;
        }
        if (se->PreloadAnm(i + 5, g_CharacterFiles[g_GameManager.sides[i].character]
                                                  [g_GameManager.sides[i].unknown30 != 0 ? 5 : 1]) == NULL)
        {
            return -1;
        }
        se = g_SpriteEngine;
    }
    return 0;
}

// Unloads the player's ANM file and frees its shot file if the game frees
// its resources. Returns 0. Static, with the player in esi (0x41ce60).
static int FreeResources(Player *player)
{
    if (FreesResources())
    {
        g_SpriteEngine->ReleaseAnm(player->sideIdx + 5);
        if (player->shtFile != NULL)
        {
            g_MemoryManager.Free(player->shtFile);
            player->shtFile = NULL;
            g_ShtFiles[player->sideIdx] = NULL;
        }
    }
    return 0;
}

void Player::RemoveChain(Player *player)
{
    if (player != NULL)
    {
        FreeResources(player);
        g_Chain.Remove(player->drawElem);
        player->drawElem = NULL;
        g_Chain.Remove(player->drawElem2);
        player->drawElem2 = NULL;
        g_Chain.Remove(player->calcElem);
        player->calcElem = NULL;
        delete player;
    }
}

PlayerHitArea *Player::AllocHitArea(D3DXVECTOR3 *pos, int framesLeft, int unknown40)
{
    PlayerHitArea *area;

    if (this->freeHitAreaCount <= 1)
    {
        area = &this->hitAreas[0x200];
    }
    else
    {
        this->freeHitAreaCount--;
        area = this->freeHitAreas[this->freeHitAreaCount];
        this->freeHitAreas[this->freeHitAreaCount] = NULL;
        this->activeHitAreas[this->activeHitAreaCount] = area;
        this->activeHitAreaCount++;
    }
    area->Reset();
    area->active = 1;
    area->x = pos->x;
    area->y = pos->y;
    area->framesLeft = framesLeft;
    area->unknown40 = unknown40;
    return area;
}

PlayerHitArea *Player::AddHitBox(D3DXVECTOR3 *pos, float width, float height, int framesLeft, int unknown40)
{
    PlayerHitArea *area = this->AllocHitArea(pos, framesLeft, unknown40);

    area->width = width * 0.5f;
    area->height = height * 0.5f;
    area->kind = 1;
    return area;
}

PlayerHitArea *Player::AddHitCircle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int framesLeft, int unknown40)
{
    PlayerHitArea *area = this->AllocHitArea(pos, framesLeft, unknown40);

    area->radius = radius;
    area->radiusGrowth = radiusGrowth;
    area->kind = 1;
    return area;
}

PlayerHitArea *Player::AddDamageBox(D3DXVECTOR3 *pos, float width, float height, int unknown28, int framesLeft, int unknown40)
{
    PlayerHitArea *area = this->AllocHitArea(pos, framesLeft, unknown40);

    area->width = width * 0.5f;
    area->height = height * 0.5f;
    area->unknown28 = unknown28;
    area->kind = 0;
    return area;
}

PlayerHitArea *Player::AddDamageCircle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int unknown28, int framesLeft, int unknown40)
{
    PlayerHitArea *area = this->AllocHitArea(pos, framesLeft, unknown40);

    area->radius = radius;
    area->radiusGrowth = radiusGrowth;
    area->unknown28 = unknown28;
    area->kind = 0;
    return area;
}

PlayerHitArea *Player::AddKind4Box(D3DXVECTOR3 *pos, float width, float height, int unknown28, int framesLeft, int unknown40)
{
    PlayerHitArea *area = this->AllocHitArea(pos, framesLeft, unknown40);

    area->width = width * 0.5f;
    area->height = height * 0.5f;
    area->unknown28 = unknown28;
    area->kind = 4;
    return area;
}

PlayerHitArea *Player::AddKind4Circle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int unknown28, int framesLeft, int unknown40)
{
    PlayerHitArea *area = this->AllocHitArea(pos, framesLeft, unknown40);

    area->radius = radius;
    area->radiusGrowth = radiusGrowth;
    area->unknown28 = unknown28;
    area->kind = 4;
    return area;
}

PlayerHitArea *Player::AddKind2Circle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int unknown28, int framesLeft, int unknown40)
{
    PlayerHitArea *area = this->AllocHitArea(pos, framesLeft, unknown40);

    area->radius = radius;
    area->radiusGrowth = radiusGrowth;
    area->unknown28 = unknown28;
    area->kind = 2;
    return area;
}

void PlayerUnknown30410::Update()
{
    if (this->timer20 > 0)
    {
        this->unknown3c = 0;
        this->timer20--;
        if (this->timer20 <= 0)
        {
            this->timer20 = 0;
            this->count = 0;
            this->unknown38 = 0;
        }
    }
    if (this->timer14 > 0)
    {
        this->timer14--;
        if (this->timer14 <= 0)
        {
            this->player->side->AddScore(this->bonus);
            this->bonus = 0;
            this->unknownC = 0;
            this->timer14 = 0;
        }
    }
}

void PlayerUnknown30410::EndCombo()
{
    this->ResetCombo();
    this->player->side->AddScore(this->bonus);
    this->bonus = 0;
    this->unknownC = 0;
    this->timer14 = 0;
}

Player::Player()
{
}

Player *Player::RegisterChain(BYTE arg, int sideIdx, int unknown20)
{
    Player *player = (Player *)g_MemoryManager.Register(new Player(), sizeof(Player), "PlayerInf");

    memset(player, 0, sizeof(Player));
    player->unknown303c8 = 0;
    player->SetRegisterChainArg(arg);
    player->side = &g_GameManager.sides[sideIdx];
    player->sideIdx = sideIdx;
    player->otherSide = &g_GameManager.sides[1 - sideIdx];
    player->unknown20 = unknown20;
    player->calcElem = g_Chain.CreateElem((ChainCallback)Player::OnUpdate);
    player->calcElem->arg = player;
    player->calcElem->addedCallback = (ChainCallback)Player::AddedCallback;
    if (g_Chain.AddToCalcChain(player->calcElem, sideIdx + 0xf) == 0)
    {
        player->drawElem = g_Chain.CreateElem((ChainCallback)Player::OnDraw);
        player->drawElem2 = g_Chain.CreateElem((ChainCallback)Player::OnDraw2);
        player->drawElem->arg = player;
        player->drawElem2->arg = player;
        g_Chain.AddToDrawChain(player->drawElem, sideIdx + 0xc);
        g_Chain.AddToDrawChain(player->drawElem2, sideIdx + 0xe);
    }
    return player;
}

void Player::SubUnknown30388(float amount)
{
    if (this->unknown30388 - amount < 1.0f)
    {
        this->unknown30388 = 1.0f;
    }
    else
    {
        this->unknown30388 -= amount;
    }
}

void FireShot(Player *player, PlayerBullet *bullet, ShotData *sd)
{
    if (sd->option == 0)
    {
        bullet->pos = player->pos;
    }
    else
    {
        bullet->pos = player->options[sd->option - 1].pos;
    }
    bullet->pos[0] += sd->offsetX;
    bullet->pos[1] += sd->offsetY;
    bullet->pos[2] = 0.495f;
    bullet->size.x = sd->sizeX;
    bullet->size.y = sd->sizeY;
    bullet->size.z = 1.0f;
    bullet->angle = sd->angle;
    bullet->speed = sd->speed;
    bullet->velocity.x = cosf(sd->angle) * sd->speed;
    bullet->velocity.y = sinf(sd->angle) * sd->speed;
    bullet->timer = 0;
    bullet->unknown470 = 0;
    bullet->type = sd->type;
    bullet->damage = sd->damage;
    bullet->script = sd->script;
    if (sd->sound >= 0)
    {
        g_SoundPlayer.QueueSoundPanned(sd->sound, player->pos.x);
    }
    player->anm->StartScript(&bullet->sprite, sd->script + 5);
    bullet->unknown46e = 0;
}

int TryFireShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame == sd->frame)
    {
        FireShot(player, bullet, sd);
        return 1;
    }
    return 0;
}

void FireShots(Player *player, int frame, int table)
{
    ShotData *shot;
    int ret;
    PlayerBullet *curBullet;
    int i;

    shot = player->shtFile->tables[table].shots;
    curBullet = player->bullets;
    for (i = 0; i < 0x80; i++, curBullet++)
    {
        if (curBullet->state != 0)
        {
            continue;
        }
    again:
        if (shot->fire != NULL)
        {
            ret = shot->fire(player, curBullet, frame, shot);
        }
        else
        {
            ret = TryFireShot(player, curBullet, frame, shot);
        }
        if (ret == 1)
        {
            curBullet->sprite.flags |= 0x2000;
            curBullet->state = 1;
            curBullet->shot = shot;
            curBullet->update = curBullet->shot->update;
            curBullet->draw = curBullet->shot->draw;
            curBullet->hit = curBullet->shot->hit;
        }
        shot++;
        if (shot->frame < 0)
        {
            return;
        }
        if (ret == 0)
        {
            goto again;
        }
    }
}

void Player::UpdateBullets()
{
    int i;
    PlayerBullet *bullet;

    if (this->side->flags & 1)
    {
        return;
    }
    bullet = this->bullets;
    for (i = 0; i < 0x80; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->update != NULL && bullet->update(this, bullet))
        {
            bullet->state = 0;
            continue;
        }
        bullet->pos[0] += g_App.gameSpeed * bullet->velocity.x;
        bullet->pos[1] += g_App.gameSpeed * bullet->velocity.y;
        if (bullet->type != 2 &&
            !g_GameManager.IsInPlayArea(bullet->pos[0], bullet->pos[1], bullet->sprite.spriteData->width,
                                        bullet->sprite.spriteData->height))
        {
            bullet->state = 0;
        }
        if (g_SpriteEngine->Step(&bullet->sprite))
        {
            bullet->state = 0;
        }
        bullet->timer++;
    }
}

void Player::DrawBullets()
{
    PlayerBullet *bullet = this->bullets;
    int i;

    for (i = 0; i < 0x80; i++, bullet++)
    {
        if (bullet->state != 1)
        {
            continue;
        }
        if (bullet->sprite.autoRotate != 0)
        {
            bullet->sprite.SetRotationZ(bullet->angle);
        }
        bullet->sprite.pos.x = g_GameManager.ArenaToScreenX(bullet->pos.x);
        bullet->sprite.pos.y = g_GameManager.ArenaToScreenY(bullet->pos.y);
        bullet->sprite.pos.z = 0.4f;
        if (bullet->unknown46e)
        {
            bullet->sprite.colorRgb[2] = 0xff;
            bullet->sprite.colorRgb[1] = 0x40;
            bullet->sprite.colorRgb[0] = 0x40;
        }
        if (bullet->draw != NULL)
        {
            bullet->draw(this, bullet);
        }
        else
        {
            g_SpriteEngine->Draw(&bullet->sprite);
        }
    }
}

void Player::DrawHitBullets()
{
    PlayerBullet *bullet = this->bullets;
    int i;

    for (i = 0; i < 0x80; i++, bullet++)
    {
        if (bullet->state != 2)
        {
            continue;
        }
        if (bullet->sprite.autoRotate != 0)
        {
            bullet->sprite.SetRotationZ(bullet->angle);
        }
        bullet->sprite.pos.x = g_GameManager.ArenaToScreenX(bullet->pos.x);
        bullet->sprite.pos.y = g_GameManager.ArenaToScreenY(bullet->pos.y);
        bullet->sprite.pos.z = 0.2f;
        if (bullet->unknown46e)
        {
            bullet->sprite.colorRgb[2] = 0xff;
            bullet->sprite.colorRgb[1] = 0x40;
            bullet->sprite.colorRgb[0] = 0x40;
        }
        g_SpriteEngine->DrawSprite(&bullet->sprite);
    }
}
