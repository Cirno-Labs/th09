// Character 13's extra attacks and callbacks (pl/pl13.cpp).

#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30450 of character 13: while unknown34 reaches (28 -
// unknownB4) * 5, sends an extra attack of type 21 from `pos` and
// takes as much off it. Returns 0 (0x44ba80).
int PlayerComboFunc44ba80(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (28 - g_GameManager.unknownB4) * 5)
    {
        g_GameManager.exAttackManager->AddAttack(21, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (28 - g_GameManager.unknownB4) * 5;
    }
    return 0;
}
