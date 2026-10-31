// Character 3's extra attacks and callbacks (pl/pl03.cpp).

#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30450 of character 3: while unknown34 reaches 70 -
// unknownB4 * 3, sends an extra attack of type 13 from `pos` and takes
// 70 off it. Returns 0 (0x443790).
int PlayerComboFunc443790(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 70 - g_GameManager.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(13, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 70;
    }
    return 0;
}
