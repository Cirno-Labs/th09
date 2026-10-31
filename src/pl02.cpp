// Character 2's extra attacks and callbacks (pl/pl02.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// An update callback of the effect table: after 30 frames, grows
// unknown84 by 0.001 up to 0.08; sets unknownC9 and puts the effect on
// its side's player. Returns 1 (0x441d30).
int EffectUpdate441d30(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknownAc >= 0x1e)
    {
        if (effect->unknown84 < 0.08f)
        {
            effect->unknown84 += 0.001f;
        }
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}

// A fire callback of the shot table (2): fires `sd` on its frame,
// standing still at its angle. Returns 1 if it fired (0x4423d0).
int ShotFire4423d0(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame == sd->frame)
    {
        FireShot(player, bullet, sd);
        bullet->velocity.x = 0.0f;
        bullet->velocity.y = 0.0f;
        bullet->angle = sd->angle;
        return 1;
    }
    return 0;
}

// The free callback of extra attack type 4: frees the data block, which
// its init allocates apart from the sprites. Returns 0 (0x442ec0).
int ExAttackFree442ec0(ExAttack *attack)
{
    if (attack->data != NULL)
    {
        g_MemoryManager.Free(attack->data);
    }
    attack->data = NULL;
    return 0;
}

// The unknown30450 of character 2: while unknown34 reaches
// (45 - unknownB4) * 2, sends an extra attack of type 3 from `pos` and
// takes as much off it. Returns 0 (0x442ee0).
int PlayerComboFunc442ee0(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (45 - g_GameManager.unknownB4) * 2)
    {
        g_GameManager.exAttackManager->AddAttack(3, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (45 - g_GameManager.unknownB4) * 2;
    }
    return 0;
}
