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
