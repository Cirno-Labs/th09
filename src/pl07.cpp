// Character 7's extra attacks and callbacks (pl/pl07.cpp).

#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30450 of character 7: while unknown34 reaches 200 -
// unknownB4 * 3, sends an extra attack of type 6 from `pos` and takes
// as much off it and 5 off unknown30, down to 0. Returns 0 (0x445d80).
int PlayerComboFunc445d80(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 200 - g_GameManager.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(6, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 200 - g_GameManager.unknownB4 * 3;
        combo->unknown30 -= 5;
        if (combo->unknown30 < 0)
        {
            combo->unknown30 = 0;
        }
    }
    return 0;
}
