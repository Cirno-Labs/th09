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
