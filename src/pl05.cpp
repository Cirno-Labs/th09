// Character 5's extra attacks and callbacks (pl/pl05.cpp).

#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30450 of character 5: while unknown34 reaches 25 -
// unknownB4 / 2, sends an extra attack of type 5 from `pos` and takes
// as much off it. Returns 0 (0x445330).
int PlayerComboFunc445330(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 25 - g_GameManager.unknownB4 / 2)
    {
        g_GameManager.exAttackManager->AddAttack(5, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 25 - g_GameManager.unknownB4 / 2;
    }
    return 0;
}
