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

