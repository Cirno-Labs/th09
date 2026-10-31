// Character 14's extra attacks and callbacks (pl/pl14.cpp).

#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

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
