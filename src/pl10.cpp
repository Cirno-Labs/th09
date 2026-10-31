// Character 10's extra attacks and callbacks (pl/pl10.cpp).

#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30450 of character 10: while unknown34 reaches (25 -
// unknownB4) * 4, sends an extra attack of type 18 from `pos` and
// takes as much off it. Returns 0 (0x449780).
int PlayerComboFunc449780(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (25 - g_GameManager.unknownB4) * 4)
    {
        g_GameManager.exAttackManager->AddAttack(18, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (25 - g_GameManager.unknownB4) * 4;
    }
    return 0;
}
