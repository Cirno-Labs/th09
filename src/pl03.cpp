// Character 3's extra attacks and callbacks (pl/pl03.cpp).

#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// A fire callback of the shot table (3): fires `sd` on its frame, going
// up at 24, and sets the player's unknown1cdc and unknown1ce0 to 0.5.
// Returns 1 if it fired (0x4432a0).
int ShotFire4432a0(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame == sd->frame)
    {
        FireShot(player, bullet, sd);
        bullet->velocity.x = 0.0f;
        bullet->velocity.y = -24.0f;
        player->unknown1cdc = player->unknown1ce0 = 0.5f;
        return 1;
    }
    return 0;
}

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
