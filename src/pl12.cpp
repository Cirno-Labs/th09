// Character 12's extra attacks and callbacks (pl/pl12.cpp).

#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "unsorted.h"

// A fire callback of the shot table (7): on the frame of `sd`, fires it
// at unknown30364 turned by its angle, or straight up once that is left
// of the side. Returns 1 if it fired (0x44ab90).
int ShotFire44ab90(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    float angle;

    if (frame == sd->frame)
    {
        angle = player->AngleFromPlayer(&player->unknown30364) + sd->angle;
        if (player->unknown30364.x < -144.0f)
        {
            angle = -1.5707964f;
        }
        FireShot(player, bullet, sd);
        bullet->velocity.x = cosf(angle) * sd->speed;
        bullet->velocity.y = sinf(angle) * sd->speed;
        bullet->angle = angle;
        return 1;
    }
    return 0;
}

// The unknown30450 of character 12: while unknown34 reaches (40 -
// unknownB4) * 4, sends an extra attack of type 20 from `pos` and
// takes as much off it. Returns 0 (0x44af80).
int PlayerComboFunc44af80(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (40 - g_GameManager.unknownB4) * 4)
    {
        g_GameManager.exAttackManager->AddAttack(20, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (40 - g_GameManager.unknownB4) * 4;
    }
    return 0;
}
