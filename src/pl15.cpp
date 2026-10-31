// Character 15's extra attacks and callbacks (pl/pl15.cpp).

#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

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
