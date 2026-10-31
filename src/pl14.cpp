// Character 14's extra attacks and callbacks (pl/pl14.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30404 of character 14: whether `pos` is within the radius
// of the player's unknown368 up or down (0x44bc80).
int PlayerPosFunc44bc80(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    float radius = player->unknown368->unknown7c;

    if (diff.y * diff.y < radius * radius)
    {
        return 1;
    }
    return 0;
}

// The unknown30450 of character 14: while unknown34 reaches 100 -
// unknownB4 * 3, sends an extra attack of type 25 from `pos` and takes
// (28 - unknownB4) * 5 off it. Returns 0 (0x44c020).
int PlayerComboFunc44c020(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 100 - g_GameManager.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(25, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (28 - g_GameManager.unknownB4) * 5;
    }
    return 0;
}

// An update callback of the effect table: grows unknown7c by 0.5 up to
// 8, sets unknownC9 and puts the effect on its side's player. Returns
// 1 (0x44c290).
int EffectUpdate44c290(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 8.0f)
    {
        effect->unknown7c += 0.5f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}
