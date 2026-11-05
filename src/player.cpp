#include "player.h"
#include "ascii.h"
#include "sprite.h"
#include "bullet.h"
#include "effect.h"
#include "gui.h"
#include "enemy.h"
#include "spellcard.h"

// Puts unknown30364 and unknown30370 back to (-999, -999, 0), clears
// unknown30380 and sets unknown1ce4 and unknown1ce8 to 1. Static, with
// the player in esi (0x41ba80).
static void ResetTargets(Player *player)
{
    player->unknown30364 = D3DXVECTOR3(-999.0f, -999.0f, 0.0f);
    player->unknown30370 = D3DXVECTOR3(-999.0f, -999.0f, 0.0f);
    player->unknown30380 = 0;
    player->unknown1ce4 = player->unknown1ce8 = 1.0f;
}

float Player::AngleFromPlayer(D3DXVECTOR3 *pos)
{
    float dx = pos->x - this->pos[0];
    float dy = pos->y - this->pos[1];

    if (dy == 0.0f && dx == 0.0f)
    {
        return 1.5707964f;
    }
    return Atan2(dy, dx);
}

float Player::AngleToPlayer(D3DXVECTOR3 *pos)
{
    float dx = this->pos[0] - pos->x;
    float dy = this->pos[1] - pos->y;

    if (dy == 0.0f && dx == 0.0f)
    {
        return 1.5707964f;
    }
    return Atan2(dy, dx);
}

// Loads the shot file at `path` into `*out`, sets its diagonal speeds,
// and turns its tables' offsets into pointers and its shots' callback
// indices into functions, as th08's LoadShotTypeFile. Returns 0, or -1
// if it does not load. Static, with `out` in esi (0x41bbe0).
static int LoadShtFile(ShtFile **out, char *path)
{
    int i;
    ShotData *shot;
    float diagonal;

    *out = (ShtFile *)OpenPath(path, NULL, 0);
    if (*out == NULL)
    {
        return -1;
    }
    diagonal = sinf(0.7853982f);
    (*out)->speedDiagonal = diagonal * (*out)->speed;
    (*out)->speedDiagonalFocused = diagonal * (*out)->speedFocused;
    for (i = 0; i < (*out)->tableCount; i++)
    {
        (*out)->tables[i].shots = (ShotData *)((int)(*out)->tables[i].shots + (int)*out);
        shot = (*out)->tables[i].shots;
        while (shot->frame >= 0)
        {
            shot->fire = g_ShotFireFuncs[(int)shot->fire];
            shot->update = g_ShotUpdateFuncs[(int)shot->update];
            shot->draw = g_ShotDrawFuncs[(int)shot->draw];
            shot->hit = g_ShotHitFuncs[(int)shot->hit];
            shot++;
        }
    }
    return 0;
}

void Player::AddUnknown30388(float amount)
{
    GameSide *side;
    int before;
    int after;

    before = (int)this->unknown30388 / 100;
    this->unknown30388 += amount;
    side = this->side;
    if (side->character == 1)
    {
        this->unknown30388 += amount * 0.15f;
    }
    if (this->unknown30388 >= 400.0f)
    {
        this->unknown30388 = 400.0f;
    }
    after = (int)this->unknown30388 / 100;
    if (before != after)
    {
        side->gui->StartUnknown66fc(after);
    }
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

int Player::CheckHitBox(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;

    start.x = pos->x - size->x;
    start.y = pos->y - size->y;
    end.x = pos->x + size->x;
    end.y = size->y + pos->y;
    if (this->hitboxMin.x > end.x || this->hitboxMin.y > end.y || this->hitboxMax.x < start.x ||
        this->hitboxMax.y < start.y)
    {
        return 0;
    }
    return 1;
}

int Player::CheckHitCircle(D3DXVECTOR3 *pos, float radius)
{
    D3DXVECTOR3 d;
    float r;

    d = this->pos - *pos;
    r = radius + this->shtFile->hitboxSize;
    if (d.x * d.x + d.y * d.y >= r * r)
    {
        return 0;
    }
    return 1;
}

int Player::TouchesGrazeBox(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;

    if (this->unknown0 != 0 && this->unknown0 != 3)
    {
        return 0;
    }
    start = *pos - *size / 2.0f;
    end = *pos + *size / 2.0f;
    if (this->grazeTestMin.x > end.x || this->grazeTestMax.x < start.x || this->grazeTestMin.y > end.y ||
        this->grazeTestMax.y < start.y)
    {
        return 0;
    }
    return 1;
}

int Player::CheckLaserHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, D3DXVECTOR3 *origin, float angle, int canGraze)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    D3DXVECTOR3 boxMin;
    D3DXVECTOR3 boxMax;

    start = this->pos - *origin;
    Rotate2D(&end, &start, -angle);
    end.z = 0.0f;
    start = end + *origin;
    boxMin = start - this->hitboxSize;
    boxMax = start + this->hitboxSize;
    start = *pos - *size;
    end = *pos + *size;
    if (!(boxMin.x > end.x || boxMax.x < start.x || boxMin.y > end.y || boxMax.y < start.y))
    {
        return 1;
    }
    if (!canGraze)
    {
        return 0;
    }
    start.x -= 48.0f;
    start.y -= 48.0f;
    end.x += 48.0f;
    end.y += 48.0f;
    if (boxMin.x > end.x || boxMax.x < start.x || boxMin.y > end.y || boxMax.y < start.y)
    {
        return 0;
    }
    return 2;
}

int Player::HandleInput()
{
    int side;
    PlayerInput *input;
    float deltaX;
    float deltaY;
    float *pos;
    int i;

    deltaX = 0.0f;
    deltaY = 0.0f;
    if (this->unknownA8 == 0)
    {
        return 0;
    }
    side = this->sideIdx;
    input = &g_PlayerInputs[side];
    if (input->IsHeld2(0x50) == 0x50)
    {
        this->direction = 5;
    }
    else if (input->IsHeld2(0x60) == 0x60)
    {
        this->direction = 7;
    }
    else if (input->IsHeld2(0x90) == 0x90)
    {
        this->direction = 6;
    }
    else if (input->IsHeld2(0xa0) == 0xa0)
    {
        this->direction = 8;
    }
    else if (input->IsHeld2(0x20))
    {
        this->direction = 2;
    }
    else if (input->IsHeld2(0x10))
    {
        this->direction = 1;
    }
    else if (input->IsHeld2(0x40))
    {
        this->direction = 3;
    }
    else if (input->IsHeld2(0x80))
    {
        this->direction = 4;
    }
    else
    {
        this->direction = 0;
    }
    this->focused = g_GameManager.config->unknownB4[this->sideIdx] == 1 ? g_PlayerInputs[side].unknown2a >= 8
                                                                         : input->IsHeld2(4) != 0;
    if (this->focused)
    {
        if (this->unknown364 == NULL)
        {
            this->unknown364 = this->side->effectManager->SpawnReservedEffect(7, &this->pos, this->sideIdx, 0xffffffff);
            this->unknown368 = this->side->effectManager->SpawnReservedEffect(
                g_CharacterValuesA10[this->side->character], &this->pos, this->sideIdx + 2, 0xffffffff);
        }
        switch (this->direction)
        {
        case 4:
            this->unknown1cd8 = 0.0f;
            deltaX = this->shtFile->speedFocused;
            break;
        case 3:
            this->unknown1cd8 = 3.1415927f;
            deltaX = -this->shtFile->speedFocused;
            break;
        case 1:
            this->unknown1cd8 = -1.5707964f;
            deltaY = -this->shtFile->speedFocused;
            break;
        case 2:
            this->unknown1cd8 = 1.5707964f;
            deltaY = this->shtFile->speedFocused;
            break;
        case 5:
            this->unknown1cd8 = -2.3561945f;
            deltaX = -this->shtFile->speedDiagonalFocused;
            deltaY = deltaX;
            break;
        case 7:
            this->unknown1cd8 = 2.3561945f;
            deltaY = this->shtFile->speedDiagonalFocused;
            deltaX = -deltaY;
            break;
        case 6:
            this->unknown1cd8 = -0.7853982f;
            deltaX = this->shtFile->speedDiagonalFocused;
            deltaY = -deltaX;
            break;
        case 8:
            this->unknown1cd8 = 0.7853982f;
            deltaX = this->shtFile->speedDiagonalFocused;
            deltaY = deltaX;
            break;
        }
    }
    else
    {
        if (this->unknown364 != NULL)
        {
            this->unknown364->sprites->SetPendingInterrupt(1);
            this->unknown364 = NULL;
            this->unknown368->active = 0;
            this->unknown368 = NULL;
        }
        switch (this->direction)
        {
        case 4:
            this->unknown1cd8 = 0.0f;
            deltaX = this->shtFile->speed;
            break;
        case 3:
            this->unknown1cd8 = 3.1415927f;
            deltaX = -this->shtFile->speed;
            break;
        case 1:
            this->unknown1cd8 = -1.5707964f;
            deltaY = -this->shtFile->speed;
            break;
        case 2:
            this->unknown1cd8 = 1.5707964f;
            deltaY = this->shtFile->speed;
            break;
        case 5:
            this->unknown1cd8 = -2.3561945f;
            deltaX = -this->shtFile->speedDiagonal;
            deltaY = deltaX;
            break;
        case 7:
            this->unknown1cd8 = 2.3561945f;
            deltaY = this->shtFile->speedDiagonal;
            deltaX = -deltaY;
            break;
        case 6:
            this->unknown1cd8 = -0.7853982f;
            deltaX = this->shtFile->speedDiagonal;
            deltaY = -deltaX;
            break;
        case 8:
            this->unknown1cd8 = 0.7853982f;
            deltaX = this->shtFile->speedDiagonal;
            deltaY = deltaX;
            break;
        }
    }
    deltaX *= this->unknown1ce4 * this->unknown1cdc;
    deltaY *= this->unknown1ce8 * this->unknown1ce0;
    if (deltaX < 0.0f && this->velocityX >= 0.0f)
    {
        this->anm->StartScript(&this->sprite, 1);
    }
    else if (deltaX == 0.0f && this->velocityX < 0.0f)
    {
        this->anm->StartScript(&this->sprite, 2);
    }
    if (deltaX > 0.0f && this->velocityX <= 0.0f)
    {
        this->anm->StartScript(&this->sprite, 3);
    }
    else if (deltaX == 0.0f && this->velocityX > 0.0f)
    {
        this->anm->StartScript(&this->sprite, 4);
    }
    this->velocityX = deltaX;
    this->velocityY = deltaY;
    this->unknown1ccc.x = deltaX * g_App.gameSpeed;
    this->unknown1ccc.y = deltaY * g_App.gameSpeed;
    pos = this->pos;
    pos[0] += this->unknown1ccc.x;
    pos[1] += this->unknown1ccc.y;
    if (pos[0] < g_GameManager.arena2[0])
    {
        pos[0] = g_GameManager.arena2[0];
    }
    else if (pos[0] > g_GameManager.arena2[0] + g_GameManager.arena2[2])
    {
        pos[0] = g_GameManager.arena2[0] + g_GameManager.arena2[2];
    }
    if (pos[1] < g_GameManager.arena2[1])
    {
        pos[1] = g_GameManager.arena2[1];
    }
    else if (pos[1] > g_GameManager.arena2[1] + g_GameManager.arena2[3])
    {
        pos[1] = g_GameManager.arena2[1] + g_GameManager.arena2[3];
    }
    this->hitboxMin = this->pos - this->hitboxSize;
    this->hitboxMax = this->pos + this->hitboxSize;
    this->grazeMin = this->pos - this->grazeSize;
    this->grazeMax = this->pos + this->grazeSize;
    this->grazeTestMin = this->pos - this->unknown1cc0;
    this->grazeTestMax = this->pos + this->unknown1cc0;
    for (i = 0; i < 4; i++)
    {
        if (this->options[i].update != NULL)
        {
            this->options[i].update(this, &this->options[i]);
            g_SpriteEngine->Step(&this->options[i].sprite);
            this->options[i].timer++;
        }
    }
    if (deltaY != 0.0f || deltaX != 0.0f)
    {
        for (i = 15; i > 0; i--)
        {
            this->posHistory[i] = this->posHistory[i - 1];
        }
        this->posHistory[0] = this->pos;
    }
    return 0;
}

// Runs the hit areas in use: one waiting unknown40 frames counts it
// down, else it counts framesLeft down and grows, and is freed when it
// runs out. The spare runs the same way and ends the list while it is
// active. Static, with the player in edi (0x41c8e0).
static void UpdateHitAreas(Player *player)
{
    int i;
    PlayerHitArea **area;
    PlayerHitArea **next;
    PlayerHitArea *spare;

    area = player->activeHitAreas;
    for (i = 0; i < player->activeHitAreaCount; i++)
    {
        if ((*area)->unknown40 > 0)
        {
            (*area)->unknown40--;
        }
        else
        {
            (*area)->framesLeft--;
            (*area)->radius += (*area)->radiusGrowth;
            (*area)->width += (*area)->widthGrowth;
            (*area)->height += (*area)->heightGrowth;
            if ((*area)->framesLeft <= 0)
            {
                (*area)->Deactivate();
                player->freeHitAreas[player->freeHitAreaCount] = *area;
                player->freeHitAreaCount++;
                for (next = area; (*next = next[1]) != NULL; next++)
                {
                }
                player->activeHitAreaCount--;
                i--;
                continue;
            }
        }
        area++;
    }
    spare = &player->hitAreas[0x200];
    if (player->hitAreas[0x200].active)
    {
        if (spare->unknown40 > 0)
        {
            spare->unknown40--;
        }
        else
        {
            spare->framesLeft--;
            spare->radius += spare->radiusGrowth;
            spare->width += spare->widthGrowth;
            spare->height += spare->heightGrowth;
            if (spare->framesLeft <= 0)
            {
                spare->Deactivate();
            }
            else
            {
                player->activeHitAreas[player->activeHitAreaCount] = spare;
            }
        }
    }
    else
    {
        player->activeHitAreas[player->activeHitAreaCount] = NULL;
    }
}

// While the round runs and the player is not dying, the second button
// starts a spell card on the other side by unknown30388, as Die does
// for character 8, using it up. Static, with the player in esi
// (0x41ca00).
static void UpdateSpellCardInput(Player *player)
{
    if (g_GameManager.gui->unknown1095c != 0)
    {
        return;
    }
    if (player->GetUnknown0() == 4)
    {
        return;
    }
    if (!g_PlayerInputs[player->sideIdx].IsHeld2(2))
    {
        return;
    }
    if (player->unknown30388 >= 400.0f)
    {
        if (player->otherSide->enemyManager->FindByUnknownType(3) == NULL)
        {
            player->otherSide->spellCard->Start(2, 2, 0, player->shtFile->spellCardNames[2]);
            player->unknown30384 = 0.0f;
            player->unknown30388 = 0.0f;
            player->StartClearCircle3();
            return;
        }
        player->otherSide->spellCard->Start(1, 1, 0, player->shtFile->spellCardNames[1]);
        player->unknown30384 = 100.0f;
        player->unknown30388 = 100.0f;
        player->StartClearCircle2();
        return;
    }
    if (player->unknown30388 >= 300.0f)
    {
        player->otherSide->spellCard->Start(1, 1, 0, player->shtFile->spellCardNames[1]);
        player->unknown30384 = 0.0f;
        player->unknown30388 = 0.0f;
        player->StartClearCircle2();
        return;
    }
    if (player->unknown30388 >= 200.0f)
    {
        player->otherSide->spellCard->Start(0, 0, 0, player->shtFile->spellCardNames[0]);
        player->unknown30384 = 0.0f;
        player->unknown30388 = 0.0f;
        player->StartClearCircle1();
    }
}

// The player comes back in from below over 30 frames of unknown303c8,
// stretched and fading in, then unknown0 goes to 3. Static, with the
// player in esi (0x41cb50).
static void UpdateRespawn(Player *player)
{
    float t;

    player->unknown3031c.unknown18 = 60;
    t = 1.0f - player->unknown303c8.AsFloat() * (1.0f / 30.0f);
    player->sprite.scaleX = t + t + 1.0f;
    player->sprite.scaleY = 1.0f - t;
    player->sprite.SetBlendMode1();
    player->unknown1cdc = player->unknown1ce0 = 1.0f;
    player->sprite.color = (player->unknown303c8.AsFrames() * 255 / 30) << 24 | 0xffffff;
    player->unknown3031c.unknown10 = 0;
    if (player->sideIdx == 0)
    {
        player->pos.x = t * -176.0f;
    }
    else
    {
        player->pos.x = t * 176.0f;
    }
    player->pos.y = t * 64.0f + 384.0f;
    if (player->unknown303c8.AsFrames() >= 30)
    {
        player->SetUnknown0(3);
        player->sprite.scaleX = player->sprite.scaleY = 1.0f;
        player->sprite.color = 0xffffffff;
        player->sprite.ClearBlendMode();
        player->pos.x = 0.0f;
        player->pos.y = 384.0f;
    }
}

// Counts unknown3031c.unknown18 down; while unknown0 is 3 the player
// blinks until unknown303c8 runs out, keeping unknown303e0 on it, else
// unknown303c8 counts up; the player blinks red while unknown1b74
// runs. Static, with the player in esi (0x41cc80).
static void UpdateInvulnerability(Player *player)
{
    if (player->unknown3031c.unknown18 != 0)
    {
        player->unknown3031c.unknown18--;
    }
    if (player->GetUnknown0() == 3)
    {
        if (player->unknown303e0 != NULL)
        {
            player->unknown303e0->pos = player->pos;
        }
        player->unknown303c8--;
        if (player->unknown303c8.AsFrames() <= 0)
        {
            if (player->unknown303e0 != NULL)
            {
                player->unknown303e0->active = 0;
                player->unknown303e0 = NULL;
            }
            player->SetUnknown0(0);
            player->unknown303c8 = 0;
            player->sprite.color = 0xffffffff;
        }
        else
        {
            player->sprite.color = player->unknown303c8.AsFrames() % 8 < 2 ? 0xfff02020 : 0xffffffff;
        }
    }
    else
    {
        player->unknown303c8++;
    }
    if (player->unknown1b74 > 0)
    {
        if (player->unknown303c8.AsFrames() % 4 < 2)
        {
            player->sprite.color = 0xfff02020;
        }
        else
        {
            player->sprite.color = 0xffffffff;
        }
        player->unknown1b74--;
        if (player->unknown1b74 <= 0)
        {
            player->unknown1b74 = 0;
            player->sprite.color = 0xffffffff;
        }
    }
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

void PlayerUnknown30410::AddCombo(D3DXVECTOR3 *pos, int a, int b, int c, int d)
{
    int prev = this->unknownC;
    int value;
    D3DXVECTOR3 effectPos;
    D3DXVECTOR3 effectVelocity;
    Effect *effect;

    if (g_GameManager.gui->GetUnknown11ea8())
    {
        return;
    }
    this->unknown3c = 0;
    this->count++;
    if (this->maxCount < this->count)
    {
        this->maxCount = this->count;
    }
    this->unknownC += g_GameManager.rank.unknownB4 * d / 10 + d;
    if (this->unknownC % 10 != 0)
    {
        this->unknownC = this->unknownC - this->unknownC % 10 + 10;
    }
    if (this->unknownC > 999990)
    {
        this->unknownC = 999990;
        this->player->side->AddScore(5000);
    }
    if (this->unknownC >= 500000 && prev < 500000)
    {
        this->player->otherSide->spellCard->Start(1, 1, 0, this->player->shtFile->spellCardNames[1]);
        if (this->player->otherSide->enemyManager->FindByUnknownType(3) == NULL)
        {
            this->player->otherSide->spellCard->Start(2, 2, 0, this->player->shtFile->spellCardNames[2]);
        }
    }
    else if (this->unknownC >= 300000 && prev < 300000)
    {
        if (this->player->otherSide->enemyManager->FindByUnknownType(3) == NULL)
        {
            this->player->otherSide->spellCard->Start(2, 2, 0, this->player->shtFile->spellCardNames[2]);
        }
    }
    else if (this->unknownC >= 100000 && prev < 100000)
    {
        this->player->otherSide->spellCard->Start(2, 2, 0, this->player->shtFile->spellCardNames[2]);
    }
    value = this->unknownC >= 999990 ? -1 : this->unknownC;
    if (this->count < 10)
    {
        if (this->count % 2 == 0)
        {
            g_AsciiText.AddPopup(this->player->sideIdx, pos, value,
                                 this->unknownC >= 300000   ? 0xffffff00
                                 : this->unknownC >= 100000 ? 0xffffffc0
                                                            : 0xffffffff);
        }
    }
    else if (this->count < 30)
    {
        if (this->count % 5 == 0)
        {
            g_AsciiText.AddPopup(this->player->sideIdx, pos, value,
                                 this->unknownC >= 300000   ? 0xffffff00
                                 : this->unknownC >= 100000 ? 0xffffffc0
                                                            : 0xffffffff);
        }
    }
    else if (this->count % 10 == 0)
    {
        g_AsciiText.AddPopup(this->player->sideIdx, pos, value,
                             this->unknownC >= 300000   ? 0xffffff00
                             : this->unknownC >= 100000 ? 0xffffffc0
                                                        : 0xffffffff);
    }
    if (a != 0)
    {
        if (g_GameManager.difficulty == 2)
        {
            a = a * 5 / 4;
        }
        else if (g_GameManager.difficulty == 3)
        {
            a = a * 4 / 3;
        }
        this->unknown2c += a;
        this->unknown30 += b;
        this->unknown34 += c;
        this->func(this, pos);
        while (this->unknown30 >= 60 - g_GameManager.rank.unknownB4 * 3 / 2)
        {
            if (this->player->otherSide->enemyManager->unknown2ac3b8 < 25)
            {
                effectPos.x = g_GameManager.ArenaToScreenX(pos->x) + g_Rng.GetRandomF32SignedInRange(32.0f);
                effectPos.y = g_GameManager.ArenaToScreenY(pos->y) + g_Rng.GetRandomF32SignedInRange(32.0f);
                effectPos.z = 0.0f;
                g_App.SetCamera(1 - this->player->sideIdx);
                effectVelocity.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 8.0f);
                effectVelocity.y = g_Rng.GetRandomF32InRange(128.0f);
                effectVelocity.z = 0.0f;
                effect = g_GameManager.sides[2].effectManager->SpawnParticlesWithVelocity(
                    this->player->sideIdx + 3, &effectPos, &effectVelocity, 1, 0xffffffff);
                g_App.SetCamera(this->player->sideIdx);
                effect->unknownA4 = 0;
                effect->unknownA6 = 1;
                effect->unknownA0 =
                    g_Rng.GetRandomF32InRange(0.5f) * 0.016666668f + g_GameManager.rank.unknownB4 * 0.0016666667f;
                effect->unknownA8 = this->player->sideIdx;
            }
            this->unknown30 += g_GameManager.rank.unknownB4 * 2 - 60;
            this->unknown2c -= 10;
            if (this->unknown2c < 0)
            {
                this->unknown2c = 0;
            }
        }
        while (this->unknown2c >= (30 - g_GameManager.rank.unknownB4) * 4)
        {
            effectPos.x = g_GameManager.ArenaToScreenX(pos->x) + g_Rng.GetRandomF32SignedInRange(32.0f);
            effectPos.y = g_GameManager.ArenaToScreenY(pos->y) + g_Rng.GetRandomF32SignedInRange(32.0f);
            effectPos.z = 0.0f;
            g_App.SetCamera(1 - this->player->sideIdx);
            effectVelocity.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 8.0f);
            effectVelocity.y = g_Rng.GetRandomF32InRange(128.0f);
            effectVelocity.z = 0.0f;
            effect = g_GameManager.sides[2].effectManager->SpawnParticlesWithVelocity(
                this->player->sideIdx + 1, &effectPos, &effectVelocity, 1, 0xffffffff);
            g_App.SetCamera(this->player->sideIdx);
            effect->unknownA4 = 0;
            effect->unknownA6 = 4;
            switch (g_GameManager.difficulty)
            {
            case 0:
                effect->unknownA0 = g_GameManager.rank.unknownB4 * 0.04f + 0.9f;
                break;
            case 1:
                effect->unknownA0 = g_GameManager.rank.unknownB4 * 0.05f + 1.1f;
                break;
            case 2:
                effect->unknownA0 = g_GameManager.rank.unknownB4 * 0.08f + 1.3f;
                break;
            case 3:
                effect->unknownA0 = g_GameManager.rank.unknownB4 * 0.11f + 1.3f;
                break;
            case 4:
                effect->unknownA0 = g_GameManager.rank.unknownB4 * 0.04f + 1.6f;
                break;
            }
            effect->unknownA8 = this->player->sideIdx;
            this->unknown2c += g_GameManager.rank.unknownB4 * 4 - 120;
        }
    }
    if (this->timer20 < 30)
    {
        this->timer20 += 30;
    }
    else if (this->timer20 < 50)
    {
        this->timer20 += 5;
    }
    else if (this->timer20 < 60)
    {
        this->timer20 += 2;
    }
    if (this->bonus <= this->unknownC)
    {
        this->bonus = this->unknownC;
        this->timer14 = this->timer20.AsFrames() + 45;
    }
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

PlayerUnknown36cEntry *PlayerUnknown36c::CheckHitsAt(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, float margin)
{
    Player *player = this->player;
    PlayerUnknown36cEntry *entry;
    int i;
    D3DXVECTOR3 savedPos;
    D3DXVECTOR3 savedSize;

    if (player->GetUnknown0() != 0)
    {
        return NULL;
    }
    if (player->unknown1b74 > 0)
    {
        return NULL;
    }
    entry = this->entries;
    savedPos = player->pos;
    savedSize = player->hitboxSize;
    player->pos = *pos;
    this->player->hitboxMin = *pos - *size;
    this->player->hitboxMax = *pos + *size;
    this->player->hitboxSize = *size;
    for (i = 0; i < this->count; i++, entry++)
    {
        if (entry->radius == 0.0f)
        {
            if (entry->angle == 0.0f)
            {
                if (this->player->CheckHitBox(&entry->pos, &entry->size))
                {
                    this->player->pos = savedPos;
                    this->player->hitboxSize = savedSize;
                    return entry;
                }
            }
            else if (this->player->CheckLaserHit(&entry->pos, &entry->size, &entry->unknownC, entry->angle, 0))
            {
                this->player->pos = savedPos;
                this->player->hitboxSize = savedSize;
                return entry;
            }
        }
        else if (this->player->CheckHitCircle(&entry->pos, margin + entry->radius))
        {
            this->player->pos = savedPos;
            this->player->hitboxSize = savedSize;
            return entry;
        }
    }
    this->player->pos = savedPos;
    this->player->hitboxSize = savedSize;
    return NULL;
}

PlayerUnknown36cEntry *PlayerUnknown36c::CheckHits()
{
    Player *player = this->player;
    PlayerUnknown36cEntry *entry;
    int i;
    int hit;

    if (player->GetUnknown0() != 0)
    {
        return NULL;
    }
    if (player->unknown1b74 > 0)
    {
        return NULL;
    }
    entry = this->entries;
    player->hitboxMin = player->pos - player->hitboxSize;
    this->player->hitboxMax = this->player->pos + this->player->hitboxSize;
    for (i = 0; i < this->count; i++, entry++)
    {
        if (entry->radius == 0.0f)
        {
            if (entry->angle == 0.0f)
            {
                hit = this->player->CheckHitBox(&entry->pos, &entry->size);
            }
            else
            {
                hit = this->player->CheckLaserHit(&entry->pos, &entry->size, &entry->unknownC, entry->angle, 0);
            }
        }
        else
        {
            hit = this->player->CheckHitCircle(&entry->pos, entry->radius);
        }
        if (hit)
        {
            return entry;
        }
    }
    return NULL;
}

void Player::AddUnknown30454(int script, D3DXVECTOR3 *pos)
{
    PlayerUnknown30454 *entry = this->unknown30454;
    int i;

    if (g_GameManager.gui->unknown1095c != 0)
    {
        return;
    }
    for (i = 0; i < 4; i++, entry++)
    {
        if (entry->unknown1c == 0)
        {
            entry->script = script;
            entry->pos = *pos;
            entry->velocity.x = 0.0f;
            entry->velocity.y = -1.5f;
            entry->velocity.z = 0.0f;
            entry->unknown1c = 1;
            this->side->bulletManager->etamaAnm->StartScriptAtOrigin(&entry->sprite, script + 0x46);
            return;
        }
    }
}

void Player::UpdateUnknown30454()
{
    PlayerUnknown30454 *entry = this->unknown30454;
    int i;
    D3DXVECTOR3 size(this->shtFile->unknown10, this->shtFile->unknown10, 16.0f);
    D3DXVECTOR3 pos;
    D3DXVECTOR3 velocity;
    Effect *effect;

    this->unknown30f64 = -1000.0f;
    for (i = 0; i < 4; i++, entry++)
    {
        if (entry->unknown1c == 0)
        {
            continue;
        }
        entry->pos += entry->velocity;
        if (entry->pos.y >= 464.0f)
        {
            entry->unknown1c = 0;
            continue;
        }
        if (entry->velocity.y < 3.0f)
        {
            entry->velocity.y += 0.03f;
        }
        else
        {
            entry->velocity.y = 3.0f;
        }
        if (this->TouchesGrazeBox(&entry->pos, &size))
        {
            switch (entry->script)
            {
            case 0:
                this->AddUnknown30388(400.0f);
                break;
            case 1:
                // i is the loop's counter too, so collecting this one ends
                // the loop.
                for (i = 0; i < g_GameManager.rank.unknownB4 + 10 + g_GameManager.difficulty * 2; i++)
                {
                    g_App.SetCamera(this->sideIdx);
                    pos.x = g_GameManager.ArenaToScreenX(this->pos.x) + g_Rng.GetRandomF32SignedInRange(64.0f);
                    pos.y = g_GameManager.ArenaToScreenY(this->pos.y) + g_Rng.GetRandomF32SignedInRange(64.0f);
                    pos.z = 0.0f;
                    g_App.SetCamera(1 - this->sideIdx);
                    velocity.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 8.0f);
                    velocity.y = g_Rng.GetRandomF32InRange(128.0f);
                    velocity.z = 0.0f;
                    effect = g_GameManager.sides[2].effectManager->SpawnParticlesWithVelocity(this->sideIdx + 1, &pos,
                                                                                              &velocity, 1, 0xffffffff);
                    effect->unknownA4 = 3;
                    effect->unknownA6 = 4;
                    switch (g_GameManager.difficulty)
                    {
                    case 0:
                        effect->unknownA0 = g_GameManager.rank.unknownB4 * 0.04f + 0.9f;
                        break;
                    case 1:
                        effect->unknownA0 = g_GameManager.rank.unknownB4 * 0.05f + 1.1f;
                        break;
                    case 2:
                        effect->unknownA0 = g_GameManager.rank.unknownB4 * 0.08f + 1.3f;
                        break;
                    case 3:
                        effect->unknownA0 = g_GameManager.rank.unknownB4 * 0.1f + 1.7f;
                        break;
                    case 4:
                        effect->unknownA0 = g_GameManager.rank.unknownB4 * 0.08f + 1.5f;
                        break;
                    }
                    effect->unknownA8 = this->sideIdx;
                    effect->unknown98 = i * 2.0f;
                }
                g_App.SetCamera(this->sideIdx);
                break;
            case 2:
                this->unknown30410.AddCombo(&this->pos, 1, 0, 400, 0);
                break;
            case 3:
                this->unknown30410.AddCombo(&this->pos, 0, 0, 0, 70000);
                break;
        }
        entry->unknown1c = 0;
        g_SoundPlayer.PlaySoundByIdx(0x1f, this->sideIdx != 0 ? 500 : -500);
        }
        else
        {
            this->unknown30f64 = entry->pos.x;
            this->unknown30f68 = 400.0f - (400.0f - entry->pos.y) * 0.5f;
        }
    }
}

void Player::DrawUnknown30454()
{
    int i;

    for (i = 0; i < 4; i++)
    {
        if (this->unknown30454[i].unknown1c != 0)
        {
            this->unknown30454[i].sprite.pos.x = g_GameManager.ArenaToScreenX(this->unknown30454[i].pos.x);
            this->unknown30454[i].sprite.pos.y = g_GameManager.ArenaToScreenY(this->unknown30454[i].pos.y);
            this->unknown30454[i].sprite.pos.z = 0.15f;
            g_SpriteEngine->DrawUnrotated(&this->unknown30454[i].sprite);
        }
    }
}

int Player::CheckHitAreas(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, Bullet *bullet)
{
    PlayerHitArea **area;
    // The point from the area's centre, then turned into its frame;
    // also the particle's position and velocity.
    D3DXVECTOR3 d;
    D3DXVECTOR3 rot;
    // A box's half size, or its corners.
    D3DXVECTOR3 half;
    D3DXVECTOR3 max;
    float dx;
    float dy;
    Effect *effect;

    for (area = this->activeHitAreas;; area++)
    {
        if (*area == NULL)
        {
            return 0;
        }
        if ((*area)->kind != 1 && (*area)->kind != 2 && (*area)->kind != 3)
        {
            continue;
        }
        if ((*area)->unknown40 > 0)
        {
            continue;
        }
        if ((*area)->radius != 0.0)
        {
            dx = pos->x - (*area)->x;
            dy = pos->y - (*area)->y;
            if (dx * dx + dy * dy < (*area)->radius * (*area)->radius)
            {
            hit:
                if ((*area)->kind != 2 && (*area)->kind != 3)
                {
                    (*area)->hits++;
                    return 2;
                }
                if (bullet != NULL && bullet->sprite == 0)
                {
                    break;
                }
            }
        }
        else if ((*area)->angle != 0.0f)
        {
            d.x = pos->x - (*area)->x;
            d.y = pos->y - (*area)->y;
            Rotate2D(&rot, &d, -(*area)->angle);
            half.x = (*area)->width;
            half.y = (*area)->height;
            if (-half.x <= rot.x && rot.x <= half.x && -half.y <= rot.y && rot.y <= half.y)
            {
                goto hit;
            }
        }
        else
        {
            half.x = (*area)->x - (*area)->width;
            half.y = (*area)->y - (*area)->height;
            max.x = (*area)->width + (*area)->x;
            max.y = (*area)->height + (*area)->y;
            if (!(half.x > pos->x || max.x < pos->x || half.y > pos->y || max.y < pos->y))
            {
                goto hit;
            }
        }
    }
    d.x = g_GameManager.ArenaToScreenX(pos->x);
    d.y = g_GameManager.ArenaToScreenY(pos->y);
    d.z = 0.0f;
    g_App.SetCamera(1 - this->sideIdx);
    rot.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 8.0f);
    rot.y = g_Rng.GetRandomF32InRange(128.0f);
    rot.z = 0.0f;
    effect = g_GameManager.sides[2].effectManager->SpawnParticlesWithVelocity(this->sideIdx + 1, &d, &rot, 1,
                                                                              0xffffffff);
    g_App.SetCamera(this->sideIdx);
    effect->unknownA4 = bullet->sprite;
    effect->unknownA6 = bullet->color;
    effect->unknownA0 = bullet->speed;
    effect->unknownA8 = bullet->mgr->sideIdx;
    effect->unknownAa = (BYTE)bullet->unknown10bd;
    this->AddUnknown30388(0.2f);
    g_SoundPlayer.PlaySoundByIdx(0x2e, this->sideIdx != 0 ? 500 : -500);
    this->unknown30410.AddCombo(pos, (g_GameManager.difficulty >= 3) + 2, 2, 0,
                                this->unknown30410.unknown38 > 100 ? 1000 : this->unknown30410.unknown38 * 10);
    this->side->AddScore(50);
    (*area)->hits++;
    return 2;
}

int Player::CheckGraze(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, Bullet *bullet)
{
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;

    if (this->CheckHitAreas(pos, size, bullet))
    {
        return 2;
    }
    start.x = pos->x - size->x / 2.0f - 60.0f;
    start.y = pos->y - size->y / 2.0f - 60.0f;
    end.x = size->x / 2.0f + pos->x + 60.0f;
    end.y = size->y / 2.0f + pos->y + 60.0f;
    if (this->grazeMin.x > end.x || this->grazeMax.x < start.x || this->grazeMin.y > end.y ||
        this->grazeMax.y < start.y)
    {
        return 0;
    }
    return 1;
}

void Player::Die()
{
    int lifeLoss;
    float angle;

    if (g_GameManager.gui->GetUnknown11ea8() != 0)
    {
        return;
    }
    g_SoundPlayer.QueueSoundPanned(4, this->pos.x);
    lifeLoss = g_GameManager.rank.unknownB8;
    if (this->unknown20 != 0)
    {
        lifeLoss += g_GameManager.rank.unknownBc;
    }
    if (this->unknownA8 <= 1)
    {
        if (this->side->character == 8 && this->unknown30388 >= 200.0f)
        {
            if (this->unknown30388 >= 400.0f)
            {
                if (this->otherSide->enemyManager->FindByUnknownType(3) == NULL)
                {
                    this->otherSide->spellCard->Start(2, 2, 0, this->shtFile->spellCardNames[2]);
                    this->unknown30384 = 0.0f;
                    this->unknown30388 = 0.0f;
                    this->StartClearCircle3();
                    return;
                }
                this->otherSide->spellCard->Start(1, 1, 0, this->shtFile->spellCardNames[1]);
                this->unknown30384 = 100.0f;
                this->unknown30388 = 100.0f;
                this->StartClearCircle2();
                return;
            }
            if (this->unknown30388 >= 300.0f)
            {
                this->otherSide->spellCard->Start(1, 1, 0, this->shtFile->spellCardNames[1]);
                this->unknown30384 = 0.0f;
                this->unknown30388 = 0.0f;
                this->StartClearCircle2();
                return;
            }
            if (this->unknown30388 >= 200.0f)
            {
                this->otherSide->spellCard->Start(0, 0, 0, this->shtFile->spellCardNames[0]);
                this->unknown30384 = 0.0f;
                this->unknown30388 = 0.0f;
                this->StartClearCircle1();
            }
            return;
        }
        this->unknownA8 = 0;
        g_GameManager.gui->CountRoundWin(1 - this->sideIdx);
        this->side->effectManager->SpawnReservedEffect(0x13, &this->pos, 4, 0xff4040ff);
    }
    else if (this->unknownA8 - lifeLoss > 1)
    {
        this->unknownA8 -= lifeLoss;
    }
    else
    {
        g_SoundPlayer.PlaySoundByIdx(0x30, this->sideIdx != 0 ? 500 : -500);
        this->side->gui->StartUnknown6ee8(0);
        this->unknownA8 = 1;
    }
    this->unknown0 = 4;
    this->unknown303c8 = 0;
    angle = g_Rng.GetRandomF32SignedInRange(3.1415927f);
    this->unknown1cd8 = angle;
    SetPolar((Vector2 *)&this->unknown1ccc, angle, 1.0f);
    g_GameManager.gui->unknownE93c[this->sideIdx] = 0x3c;
    g_GameManager.gui->unknownE934[this->sideIdx] = 0x80ff0000;
    if (this->unknown364 != NULL)
    {
        this->unknown364->sprites->SetPendingInterrupt(1);
        this->unknown364 = NULL;
        this->unknown368->active = 0;
        this->unknown368 = NULL;
    }
    g_GameManager.rank.LowerUnknownCc(1500);
    if (g_GameManager.rank.unknownB8 > 4)
    {
        g_GameManager.rank.unknownB8 -= 2;
    }
    else if (g_GameManager.rank.unknownB8 > 2)
    {
        g_GameManager.rank.unknownB8--;
    }
    this->unknown30410.EndCombo();
    this->unknown24.Reset();
}

// The player is blown along unknown1ccc, slowing over 60 frames of
// unknown303c8 and kept in arena2, fading out; then it gets
// unknown30388 back by its lives, unknown0 goes to 3 and it clears
// around itself. Returns 0. Static, with the player in esi (0x41e720).
static int UpdateDying(Player *player)
{
    float *pos;
    float amount;

    if (player->unknown303c8.AsFloat() < 60.0f)
    {
        player->pos += player->unknown1ccc * (60.0f - player->unknown303c8.AsFloat()) / 20.0f;
        pos = player->pos;
        if (pos[0] < g_GameManager.arena2[0])
        {
            pos[0] = g_GameManager.arena2[0];
        }
        else if (pos[0] > g_GameManager.arena2[2] + g_GameManager.arena2[0])
        {
            pos[0] = g_GameManager.arena2[2] + g_GameManager.arena2[0];
        }
        if (pos[1] < g_GameManager.arena2[1])
        {
            pos[1] = g_GameManager.arena2[1];
        }
        else if (pos[1] > g_GameManager.arena2[3] + g_GameManager.arena2[1])
        {
            pos[1] = g_GameManager.arena2[3] + g_GameManager.arena2[1];
        }
        player->sprite.color = (player->unknown303c8.AsFrames() * 255 / 10) << 24 | 0xffffff;
        return 0;
    }
    if (player->unknownA8 == 1)
    {
        amount = 400.0f;
    }
    else
    {
        amount = 130.0f - player->unknownA8 * 10.0f;
    }
    player->AddUnknown30388(amount);
    player->SetUnknown0(3);
    player->unknown303c8 = 60;
    player->AddHitCircle(&player->pos, 16.0f, 20.0f, 8, 0);
    player->AddKind4Circle(&player->pos, 16.0f, 20.0f, 10, 8, 0);
    return 0;
}

// While unknown0 is 0 and unknown1b74 is not running, a hit of
// unknown36c kills the player, the bullet behind it set to state 5.
// Returns whether it did. Static, with the player in esi (0x41e8b0).
static int CheckDeath(Player *player)
{
    PlayerUnknown36cEntry *entry;

    if (player->GetUnknown0() != 0 || player->unknown1b74 > 0)
    {
        return 0;
    }
    entry = player->unknown36c.CheckHits();
    if (entry == NULL)
    {
        return 0;
    }
    if (entry->unknown2c != NULL)
    {
        ((Bullet *)entry->unknown2c)->state = 5;
    }
    player->Die();
    return 1;
}

int Player::OnUpdate(Player *player)
{
    GameSide *otherSide;

    if (g_GameManager.stateFlags & 0x1800)
    {
        return 1;
    }
    if (player->side->flag0 && player->side->character != 2)
    {
        return 1;
    }
    if (!player->side->flag0)
    {
        player->UpdateUnknown30454();
    }
    if (player->GetUnknown0() == 5)
    {
        return 1;
    }
    g_App.SetCamera(player->sideIdx);
    if (player->GetUnknown0() == 4)
    {
        if (UpdateDying(player))
        {
            goto respawn;
        }
    }
    else if (player->GetUnknown0() == 2)
    {
        if (Player::UpdateUnknown0Is2())
        {
            goto respawn;
        }
    }
    else if (player->GetUnknown0() == 1)
    {
    respawn:
        UpdateRespawn(player);
    }
    UpdateInvulnerability(player);
    if (player->unknown20 > 0)
    {
        player->unknown24.Run();
    }
    UpdateHitAreas(player);
    if (!g_GameManager.gui->IsMsgRunning())
    {
        UpdateSpellCardInput(player);
    }
    if (player->GetUnknown0() != 2 && player->GetUnknown0() != 4 && player->GetUnknown0() != 1)
    {
        player->HandleInput();
    }
    g_SpriteEngine->Step(&player->sprite);
    player->UpdateBullets();
    if (!g_GameManager.gui->IsMsgRunning())
    {
        player->unknown30410.unknown3c++;
        if (player->unknownFlag3)
        {
            g_SoundPlayer.PlaySoundByIdx(0x30, player->sideIdx != 0 ? 500 : -500);
            player->side->gui->StartUnknown6ee8(0);
            player->unknownFlag3 = 0;
            if (g_GameManager.IsMode1() && player->sideIdx == 0)
            {
                otherSide = player->otherSide;
                player->side->gui->StartUnknown76d4(
                    g_PlayerCpuLevels[otherSide->unknown2c].seconds5c[2] * 60 -
                    otherSide->player->unknown24.timer5c.AsFrames());
            }
        }
        if (g_GameManager.gui->unknown1095c == 0)
        {
            player->UpdateShotInput();
        }
    }
    if (!player->side->flag0 && g_GameManager.gui->unknown1095c == 0)
    {
        CheckDeath(player);
    }
    ResetTargets(player);
    if (!player->side->flag0)
    {
        player->unknown30410.Update();
    }
    player->unknown36c.count = 0;
    return 1;
}

int Player::OnDraw(Player *player)
{
    g_App.SetView2D(player->sideIdx);
    if (player->GetUnknown0() != 5)
    {
        player->DrawBullets();
        if (player->unknownA8 == 0 &&
            (g_GameManager.gui->unknown1095c >= 2 || g_GameManager.gui->unknown10960 > 30))
        {
            return 1;
        }
        if (!g_GameManager.isInContinueMenu)
        {
            player->sprite.pos.x = g_GameManager.ArenaToScreenX(player->pos.x);
            player->sprite.pos.y = g_GameManager.ArenaToScreenY(player->pos.y);
            player->sprite.pos.z = 0.1f;
            g_SpriteEngine->DrawUnrotated(&player->sprite);
        }
    }
    player->DrawUnknown30454();
    return 1;
}

void Player::Reset()
{
    int i;
    int level;
    float *pos;

    this->anm->StartScript(&this->sprite, 0);
    pos = this->pos;
    i = 0;
    pos[i++] = -160.0f;
    pos[i++] = g_GameManager.arena[3] - 64.0f;
    pos[i] = 0.49f;
    for (i = 0; i < 0x200; i++)
    {
        this->hitAreas[i].Reset();
        this->freeHitAreas[i] = &this->hitAreas[i];
        this->activeHitAreas[i] = NULL;
    }
    this->activeHitAreaCount = 0;
    this->freeHitAreaCount = 0x200;
    this->direction = 0;
    this->unknown30410.unknown3c = 0;
    this->SetUnknown0(5);
    this->unknown303c8 = 120;
    this->unknown1b84 = 1;
    for (i = 0; i < 0x80; i++)
    {
        this->bullets[i].state = 0;
    }
    this->unknown1cdc = this->unknown1ce0 = 1.0f;
    this->unknown1ce4 = this->unknown1ce8 = 1.0f;
    for (i = 0; i < 0x10; i++)
    {
        this->posHistory[i] = this->pos;
    }
    this->unknown303e8 = 0;
    this->focused = 2;
    this->unknown303b0 = 6;
    this->unknown24.unknown58 = g_Rng.GetRandomF32InRange(400.0f) + 100.0f;
    if (this->unknown24.unknown58 >= 400.0f)
    {
        this->unknown24.unknown58 = 400.0f;
    }
    this->unknown30410.EndCombo();
    this->unknown1b74 = 0;
    this->unknown30384 = 0.0f;
    this->unknownA8 = g_GameManager.initialLives[this->sideIdx];
    level = g_GameManager.unknown100;
    if (level > 2)
    {
        level = 2;
    }
    switch (g_GameManager.difficulty)
    {
    case 0:
    case 1:
        this->unknownA4 = this->unknownA0 = level + 1;
        break;
    case 2:
        this->unknownA4 = this->unknownA0 = level + 4;
        break;
    case 3:
        this->unknownA4 = this->unknownA0 = level + 8;
        break;
    case 4:
        this->unknownA4 = this->unknownA0 = level + 12;
        this->unknownA8 = 1;
        this->unknown30388 = 400.0f;
        break;
    }
    if (this->unknownA8 == 1)
    {
        this->unknownFlag3 = 1;
    }
    this->unknown303a4 = -1;
    this->unknown1cd8 = -1.5707964f;
    this->unknown364 = NULL;
    this->unknown24.Reset();
    memset(this->unknown30454, 0, sizeof(this->unknown30454));
}

int Player::AddedCallback(Player *player)
{
    GameSide *side;

    if (g_ShtFiles[player->sideIdx] != NULL && player->side->character == player->side->unknown24)
    {
        player->shtFile = g_ShtFiles[player->sideIdx];
    }
    else
    {
        if (g_ShtFiles[player->sideIdx] != NULL)
        {
            g_MemoryManager.Free(g_ShtFiles[player->sideIdx]);
        }
        if (LoadShtFile(&player->shtFile, g_CharacterFiles[player->side->character][0]))
        {
            return -1;
        }
        g_ShtFiles[player->sideIdx] = player->shtFile;
    }
    side = player->side;
    player->unknown303f0 = g_CharacterValuesBd0[side->character];
    player->unknown303f4 = g_CharacterValuesB90[side->character];
    player->unknown303f8 = g_CharacterValuesB50[side->character];
    player->unknown303fc = g_CharacterValuesB10[side->character];
    player->unknown30400 = g_CharacterValuesAd0[side->character];
    player->anm = g_SpriteEngine->GetAnm(player->sideIdx + 5);
    player->hitboxSize.x = player->hitboxSize.y = player->shtFile->hitboxSize / 2.0f;
    player->hitboxSize.z = 5.0f;
    player->grazeSize.x = player->grazeSize.y = player->shtFile->grazeSize / 2.0f;
    player->grazeSize.z = 5.0f;
    player->unknown1cc0.x = player->unknown1cc0.y = player->shtFile->unknown10 / 2.0f;
    player->unknown1cc0.z = 5.0f;
    player->unknown30388 = 100.0f;
    player->unknown30410.player = player;
    player->unknown30410.func = g_CharacterFuncs9d0[side->character];
    player->unknownA4 = player->unknownA0 = 1;
    player->unknown24.player = player;
    player->unknown36c.player = player;
    player->unknownAc = 2;
    player->unknownB8 = player->unknownB4 = player->unknownB0 = 0;
    player->unknown30404 = g_CharacterFuncsA50[side->character];
    player->unknown30408 = g_CharacterFuncsA90[side->character];
    player->Reset();
    return 0;
}

Player::Player()
{
}

Player *Player::RegisterChain(int arg, int sideIdx, int unknown20)
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

// The corners of a `size` box centred on `pos`, as th08's inline.
static void GetBox(D3DXVECTOR3 &min, D3DXVECTOR3 &max, D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    min.x = pos->x - size->x / 2.0f;
    min.y = pos->y - size->y / 2.0f;
    max.x = size->x / 2.0f + pos->x;
    max.y = size->y / 2.0f + pos->y;
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

int Player::UpdateShotInput()
{
    int side;
    BYTE chargeType;
    int before;

    side = this->sideIdx;
    if ((g_PlayerInputs[side].IsJustPressed2(1) && !(g_GameManager.config->unknownB4[side] == 1 && g_PlayerInputs[side].IsHeld2(4))) ||
        ((chargeType = g_GameManager.config->unknownB4[side]) == 1 && g_PlayerInputs[side].IsHeld2(1) &&
         !g_PlayerInputs[side].IsHeld2(4)))
    {
        if (this->unknown303b0 == 0)
        {
            if (this->unknown303a4 < 0)
            {
                this->unknown303a4 = 0;
            }
            else if (this->unknown303a4 >= 10)
            {
                this->unknown303a4 -= 10;
            }
            else if (this->unknown303a4 >= 5)
            {
                this->unknown303a4 -= 5;
            }
            this->unknown303b0 = 3;
            this->unknown303bc = 1;
        }
        else if (this->unknown303b0 > 0)
        {
            this->unknown303b0--;
        }
        if (g_GameManager.config->unknownB4[this->sideIdx] != 1)
        {
            goto fire;
        }
    }
    else if ((chargeType == 1 && g_PlayerInputs[side].IsHeld2(4)) || (chargeType == 0 && g_PlayerInputs[side].IsHeld2(1)))
    {
        this->unknown303bc++;
        if (this->unknown303bc >= 10 && !this->charging)
        {
            if (this->unknown30384 == 0.0f)
            {
                g_SoundPlayer.PlaySoundByIdx(0x2b, this->sideIdx != 0 ? 500 : -500);
                this->side->gui->StartUnknown5c6c();
            }
            before = (int)this->unknown30384 / 100;
            this->unknown30384 += g_App.gameSpeed * this->shtFile->unknown24;
            if (this->unknown30384 >= this->unknown30388)
            {
                this->unknown30384 = this->unknown30388;
                this->unknown3038c++;
                if (this->unknown3038c >= 300)
                {
                    goto release;
                }
            }
            else
            {
                this->unknown3038c = 0;
            }
            if (before < (int)this->unknown30384 / 100)
            {
                g_SoundPlayer.PlaySoundByIdx(0x32, this->sideIdx != 0 ? 500 : -500);
            }
        }
        goto fire;
    }
release:
    if (this->unknown30384 >= 100.0f)
    {
        this->charging = 1;
        this->unknown30398 = 0;
        this->unknown1b74 = 8;
        if (this->unknown30384 >= 400.0f && this->otherSide->enemyManager->FindByUnknownType(3) == NULL)
        {
            this->otherSide->spellCard->Start(2, 2, 0, this->shtFile->spellCardNames[2]);
            this->SubUnknown30388(300.0f);
            this->unknown1b74 = 8;
            this->StartClearCircle3();
        }
        else if (this->unknown30384 >= 300.0f)
        {
            this->otherSide->spellCard->Start(1, 1, 0, this->shtFile->spellCardNames[1]);
            this->SubUnknown30388(200.0f);
            this->unknown1b74 = 8;
            this->StartClearCircle2();
        }
        else if (this->unknown30384 >= 200.0f)
        {
            this->otherSide->spellCard->Start(0, 0, 0, this->shtFile->spellCardNames[0]);
            this->unknown1b74 = 8;
            this->SubUnknown30388(100.0f);
            this->StartClearCircle1();
        }
    }
    this->side->gui->StopUnknown5c6c();
    this->unknown303bc = -1;
    this->unknown30384 = 0.0f;
    if (this->unknown303b0 > 0)
    {
        this->unknown303b0--;
    }
fire:
    if (this->charging)
    {
        if (this->unknown30398.HasTicked())
        {
            FireShots(this, this->unknown30398.AsFrames(), 1);
        }
        this->unknown30398++;
        if (this->unknown30398.AsFloat() >= this->shtFile->unknown28)
        {
            this->unknown30398 = 0;
            this->charging = 0;
        }
    }
    if (this->unknown303a4 >= 0)
    {
        if (this->unknown303a4.HasTicked())
        {
            FireShots(this, this->unknown303a4.AsFrames(), 0);
        }
        this->unknown303a4++;
        if (this->unknown303a4 >= 15)
        {
            this->unknown303a4 = -1;
        }
    }
    return 0;
}

int Player::CalcDamage(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, int *damageTaken, int *hitByBomb, int *kind4Damage)
{
    PlayerBullet *cur;
    D3DXVECTOR3 enemyMin;
    D3DXVECTOR3 enemyMax;
    int idx;
    D3DXVECTOR3 targetLo;
    D3DXVECTOR3 targetHi;
    float rotZ;
    int damage;
    PlayerHitArea **area;

    damage = 0;
    if (!this->unknown303c8.HasTicked())
    {
        return 0;
    }
    GetBox(enemyMin, enemyMax, pos, size);
    cur = this->bullets;
    if (hitByBomb != NULL)
    {
        *hitByBomb = 0;
    }
    for (idx = 0; idx < 0x80; idx++, cur++)
    {
        if (cur->state == 0 || cur->state != 1)
        {
            continue;
        }
        GetBox(targetLo, targetHi, &cur->pos, &cur->size);
        if (targetLo.y > enemyMax.y || targetLo.x > enemyMax.x || targetHi.y < enemyMin.y || targetHi.x < enemyMin.x)
        {
            continue;
        }
        if (cur->type == 2 && cur->timer % 2 != 0)
        {
            continue;
        }
        if (cur->hit != NULL && cur->hit(this, cur, pos))
        {
            continue;
        }
        damage += cur->damage;
        if (cur->type != 2 && cur->type != 3)
        {
            if (cur->state == 1)
            {
                rotZ = cur->sprite.rotation.z;
                this->anm->StartScript(&cur->sprite, cur->script + 6);
                cur->sprite.rotation.z = rotZ;
                cur->pos[2] = 0.1f;
            }
            cur->state = 2;
            cur->velocity.x /= 8.0f;
            cur->velocity.y /= 8.0f;
            if (cur->type == 4)
            {
                this->side->effectManager->SpawnReservedEffect(0x22, &this->bullets->pos, 6, 0xffffffff);
                this->AddDamageCircle(&this->bullets->pos, 0.0f, 1.6f, 6, 0x46, 0);
                g_SoundPlayer.PlaySoundByIdx(0x10, this->sideIdx != 0 ? 500 : -500);
            }
        }
    }
    *damageTaken = damage;
    for (area = this->activeHitAreas; *area != NULL; area++)
    {
        if ((*area)->kind != 0 && (*area)->kind != 2 && (*area)->kind != 4)
        {
            continue;
        }
        if ((*area)->unknown40 > 0)
        {
            continue;
        }
        if ((*area)->framesLeft % (*area)->unknown34 != 0)
        {
            continue;
        }
        if ((*area)->radius == 0.0f)
        {
            if ((*area)->angle == 0.0f)
            {
                if ((*area)->x - (*area)->width > enemyMax.x || (*area)->width + (*area)->x < enemyMin.x ||
                    (*area)->y - (*area)->height > enemyMax.y || (*area)->height + (*area)->y < enemyMin.y)
                {
                    continue;
                }
            }
            else
            {
                targetLo.x = pos->x - (*area)->x;
                targetLo.y = pos->y - (*area)->y;
                Rotate2D(&targetHi, &targetLo, -(*area)->angle);
                if (-(*area)->width > size->x / 2.0f + targetHi.x || (*area)->width < targetHi.x - size->x / 2.0f ||
                    -(*area)->height > size->y / 2.0f + targetHi.y || (*area)->height < targetHi.y - size->y / 2.0f)
                {
                    continue;
                }
            }
        }
        else if ((*area)->radius * (*area)->radius <
                 ((*area)->x - pos->x) * ((*area)->x - pos->x) + ((*area)->y - pos->y) * ((*area)->y - pos->y))
        {
            continue;
        }
        damage += (*area)->unknown28;
        (*area)->hits += (*area)->unknown28;
        if ((*area)->maxDamage > 0 && (*area)->maxDamage <= (*area)->hits)
        {
            (*area)->unknown28 = 0;
            damage -= (*area)->hits - (*area)->maxDamage;
        }
        if ((*area)->kind == 4)
        {
            *kind4Damage += (*area)->unknown28;
        }
    }
    return damage;
}
