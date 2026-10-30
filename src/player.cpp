#include "player.h"
#include "sprite.h"

void Sprite::SetBlendMode1()
{
    this->blendMode = 1;
}

void Sprite::ClearBlendMode()
{
    this->blendMode = 0;
}

AnmFile *SpriteEngine::GetAnmFile(int slot)
{
    return this->GetAnm(slot)->file;
}

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
