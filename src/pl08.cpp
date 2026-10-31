// Character 8's extra attacks and callbacks (pl/pl08.cpp).

#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30450 of character 8: while unknown34 reaches 100 -
// unknownB4 * 3, sends an extra attack of type 16 from `pos` and takes
// as much off it. Returns 0 (0x447a80).
int PlayerComboFunc447a80(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 100 - g_GameManager.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(16, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 100 - g_GameManager.unknownB4 * 3;
    }
    return 0;
}
