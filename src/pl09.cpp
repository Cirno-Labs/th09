// Character 9's extra attacks and callbacks (pl/pl09.cpp).

#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30450 of character 9: while unknown34 reaches (30 -
// unknownB4) * 2, sends an extra attack of type 17 from `pos` and
// takes as much off it. Returns 0 (0x448640).
int PlayerComboFunc448640(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (30 - g_GameManager.unknownB4) * 2)
    {
        g_GameManager.exAttackManager->AddAttack(17, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (30 - g_GameManager.unknownB4) * 2;
    }
    return 0;
}
