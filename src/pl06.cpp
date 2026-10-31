// Character 6's extra attacks and callbacks (pl/pl06.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30404 of character 6: whether `pos` is within the radius
// of the player's unknown368 across (0x445580).
int PlayerPosFunc445580(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    float radius = player->unknown368->unknown7c;

    if (diff.x * diff.x < radius * radius)
    {
        return 1;
    }
    return 0;
}

// The unknown30450 of character 6: while unknown34 reaches 100 -
// unknownB4 * 3, sends an extra attack of type 15 from `pos` and takes
// (28 - unknownB4) * 5 off it. Returns 0 (0x445930).
int PlayerComboFunc445930(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 100 - g_GameManager.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(15, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (28 - g_GameManager.unknownB4) * 5;
    }
    return 0;
}
