// Character 4's extra attacks and callbacks (pl/pl04.cpp).

#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30450 of character 4: while unknown34 reaches (28 -
// unknownB4) * 5, sends an extra attack of type 14 from `pos` and
// takes as much off it. Returns 0 (0x4444f0).
int PlayerComboFunc4444f0(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (28 - g_GameManager.unknownB4) * 5)
    {
        g_GameManager.exAttackManager->AddAttack(14, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (28 - g_GameManager.unknownB4) * 5;
    }
    return 0;
}
