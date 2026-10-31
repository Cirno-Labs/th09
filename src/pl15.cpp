// Character 15's extra attacks and callbacks (pl/pl15.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30404 of character 15: whether `pos` is within the radius
// of the player's unknown368 across or up or down (0x44c2e0).
int PlayerPosFunc44c2e0(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;

    if (diff.x * diff.x < player->unknown368->unknown7c * player->unknown368->unknown7c ||
        diff.y * diff.y < player->unknown368->unknown7c * player->unknown368->unknown7c)
    {
        return 1;
    }
    return 0;
}

// The unknown30450 of character 15: while unknown34 reaches 80 -
// unknownB4 * 3, sends an extra attack of type 26 from `pos` and takes
// as much off it. Returns 0 (0x44c680).
int PlayerComboFunc44c680(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 80 - g_GameManager.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(26, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 80 - g_GameManager.unknownB4 * 3;
    }
    return 0;
}
