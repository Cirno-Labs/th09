// Character 5's extra attacks and callbacks (pl/pl05.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30450 of character 5: while unknown34 reaches 25 -
// unknownB4 / 2, sends an extra attack of type 5 from `pos` and takes
// as much off it. Returns 0 (0x445330).
int PlayerComboFunc445330(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 25 - g_GameManager.unknownB4 / 2)
    {
        g_GameManager.exAttackManager->AddAttack(5, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 25 - g_GameManager.unknownB4 / 2;
    }
    return 0;
}

// The update callback of effect type 27: grows unknown7c by 0.5 up to 16,
// sets unknownC9 and puts the effect on its side's player. Returns 1
// (0x4454e0).
int EffectUpdate4454e0(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 16.0f)
    {
        effect->unknown7c += 0.5f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}
