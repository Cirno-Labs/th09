// Character 7's extra attacks and callbacks (pl/pl07.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30450 of character 7: while unknown34 reaches 200 -
// unknownB4 * 3, sends an extra attack of type 6 from `pos` and takes
// as much off it and 5 off unknown30, down to 0. Returns 0 (0x445d80).
int PlayerComboFunc445d80(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 200 - g_GameManager.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(6, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 200 - g_GameManager.unknownB4 * 3;
        combo->unknown30 -= 5;
        if (combo->unknown30 < 0)
        {
            combo->unknown30 = 0;
        }
    }
    return 0;
}

// The unknown30404 of character 7: whether `pos` is within the radius of
// the player's unknown368 about a point 1.5 radii above the player
// (0x445e60).
int PlayerPosFunc445e60(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    Effect *circle = player->unknown368;
    float dy = diff.y - circle->unknown7c * 1.5f;

    if (diff.x * diff.x + dy * dy < circle->unknown7c * circle->unknown7c)
    {
        return 1;
    }
    return 0;
}

// An update callback of the effect table: grows unknown7c by 8 up to
// 320, sets unknownC9, puts the effect on its side's player and clears
// unknown80. Returns 1 (0x4475c0).
int EffectUpdate4475c0(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 320.0f)
    {
        effect->unknown7c += 8.0f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    effect->unknown80 = 0.0f;
    return 1;
}
