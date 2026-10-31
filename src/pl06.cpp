// Character 6's extra attacks and callbacks (pl/pl06.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"

// A fire callback of the shot table (4): on the frame of `sd`, starts
// effect 0x23 at the player in reserved slot 5. Returns 0 (0x445530).
int ShotFire445530(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame == sd->frame)
    {
        player->side->effectManager->SpawnReservedEffect(0x23, &player->pos, 5, 0xffffffff);
    }
    return 0;
}

// The unknown30404 of character 6: whether `pos` is within the radius
// of the player's unknown368 across (0x445580).
int PlayerPosFunc445580(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    float radius = player->unknown368->unknown7c;

    if (diff.x * diff.x < radius * radius)
    {
        return 1;
    }
    return 0;
}

// The unknown30450 of character 6: while unknown34 reaches 100 -
// unknownB4 * 3, sends an extra attack of type 15 from `pos` and takes
// (28 - unknownB4) * 5 off it. Returns 0 (0x445930).
int PlayerComboFunc445930(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 100 - g_GameManager.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(15, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (28 - g_GameManager.unknownB4) * 5;
    }
    return 0;
}

// The init callback of effect type 0x23: starts script 7 of the player's
// ANM on the first sprite, without writing depth and with no offset.
// Returns 0 (0x445c30).
int EffectInit445c30(Effect *effect)
{
    effect->mgr->side->player->anm->StartScript(effect->sprites, 7);
    effect->sprites->zWriteDisabled = 1;
    effect->sprites->posOffset.x = 0.0f;
    effect->sprites->posOffset.y = 0.0f;
    effect->sprites->posOffset.z = 0.0f;
    return 0;
}

// The update callback of effect type 0x23: fires the player's shots of
// table 2 on the effect's frame. Returns 1 (0x445c80).
int EffectUpdate445c80(Effect *effect)
{
    FireShots(effect->mgr->side->player, effect->unknownAc.AsFrames(), 2);
    return 1;
}

// A fire callback of the shot table (5): on the frame of `sd`, fires it
// unless the effect in reserved slot 5 is running. Returns 1 if it fired
// (0x445cb0).
int ShotFire445cb0(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (!player->side->effectManager->GetReservedEffect(5)->active && frame == sd->frame)
    {
        FireShot(player, bullet, sd);
        return 1;
    }
    return 0;
}
