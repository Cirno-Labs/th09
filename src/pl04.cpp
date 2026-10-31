// Character 4's extra attacks and callbacks (pl/pl04.cpp).

#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30408 of character 4: pl01's, rising up to 1.5 over the 120
// frames. Returns 0 (0x443c10).
int PlayerEnemyFunc443c10(Player *player, Enemy *enemy)
{
    enemy->velocity2.x = g_Rng.GetRandomF32SignedInRange(0.1f);
    enemy->velocity2.y = enemy->unknown5424 > 0x78 ? -1.5f : enemy->unknown5424.AsFloat() * (-1.5f / 120.0f);
    enemy->velocity2.z = 0.0f;
    enemy->unknown5424++;
    return 0;
}

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
