// Character 0's spell cards, extra attacks and callbacks (pl/pl00.cpp).

#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "spellcard.h"

// The update callback of spell card types 2, 5 and 8: whether no enemy of
// unknownType 3 is left on the side (0x440cc0).
int SpellCardUpdate440cc0(SpellCard *card)
{
    return g_GameManager.sides[card->sideIdx].enemyManager->FindByUnknownType(3) == NULL;
}

// An update callback of the effect table: grows unknown7c by 4 up to 96,
// sets unknownC9 and puts the effect on its side's player. Returns 1
// (0x440ce0).
int EffectUpdate440ce0(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 96.0f)
    {
        effect->unknown7c += 4.0f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}

// The unknown30404 of characters 0, 3, 5, 12 and 13: whether `pos` is
// within the radius of the player's unknown368 (0x440df0).
int PlayerPosFunc440df0(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    float radius = player->unknown368->unknown7c;

    if (diff.x * diff.x + diff.y * diff.y < radius * radius)
    {
        return 1;
    }
    return 0;
}

// The init callbacks of spell card types 0 and 1: allocate unknown1360
// and spawn enemy subroutine 0 or 1 from the player's ECL (0x441350,
// 0x4413d0). The position is never set. Return 0.
int SpellCardInit441350(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unknown1360 = g_MemoryManager.Alloc(0x2b8, ".\\pl\\pl00.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(0, &pos, 0x1e, -2, 1000, 0);
    return 0;
}

// The free callback of spell card types 0, 1, 3, 4, 6 and 7: frees
// unknown1360. Returns 0 (0x4413a0).
int SpellCardFree4413a0(SpellCard *card)
{
    if (card->unknown1360 != NULL)
    {
        g_MemoryManager.Free(card->unknown1360);
    }
    card->unknown1360 = NULL;
    return 0;
}

int SpellCardInit4413d0(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unknown1360 = g_MemoryManager.Alloc(0x2b8, ".\\pl\\pl00.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(1, &pos, 0x1e, -2, 10000, 0);
    return 0;
}

// The init callback of spell card type 2: allocates unknown1364 and
// spawns enemy subroutine 2 from the player's ECL, with the position
// never set. Returns 0 (0x441420).
int SpellCardInit441420(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unknown1364 = g_MemoryManager.Alloc(0x2b8, ".\\pl\\pl00.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(2, &pos, 0x578, -2, 1000, 0);
    return 0;
}

// The free callback of spell card types 2, 5 and 8: frees unknown1364.
// Returns 0 (0x441470).
int SpellCardFree441470(SpellCard *card)
{
    if (card->unknown1364 != NULL)
    {
        g_MemoryManager.Free(card->unknown1364);
    }
    card->unknown1364 = NULL;
    return 0;
}
