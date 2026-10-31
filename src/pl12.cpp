// Character 12's extra attacks and callbacks (pl/pl12.cpp).

#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30450 of character 12: while unknown34 reaches (40 -
// unknownB4) * 4, sends an extra attack of type 20 from `pos` and
// takes as much off it. Returns 0 (0x44af80).
int PlayerComboFunc44af80(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (40 - g_GameManager.unknownB4) * 4)
    {
        g_GameManager.exAttackManager->AddAttack(20, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (40 - g_GameManager.unknownB4) * 4;
    }
    return 0;
}
