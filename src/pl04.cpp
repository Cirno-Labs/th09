// Character 4's extra attacks and callbacks (pl/pl04.cpp).

#include "enemy.h"
#include "effect.h"
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

// An update callback of the shot table (14): while unknown470 is 1,
// stops the bullet, and frees it once it is 90 frames old. Sets
// unknown472. Returns 0 (0x4446a0).
int ShotUpdate4446a0(Player *player, PlayerBullet *bullet)
{
    if (bullet->unknown470 == 1)
    {
        bullet->velocity.x = 0.0f;
        bullet->velocity.y = 0.0f;
        if (bullet->timer >= 90)
        {
            bullet->state = 0;
        }
    }
    bullet->unknown472 = 1;
    return 0;
}

// An update callback of the effect table: grows unknown7c by 8 up to
// 48, sets unknownC9 and puts the effect on its side's player. Returns
// 1 (0x444eb0).
int EffectUpdate444eb0(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 48.0f)
    {
        effect->unknown7c += 8.0f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}
