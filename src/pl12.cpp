// Character 12's extra attacks and callbacks (pl/pl12.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "unsorted.h"

// The update callback of effect type 32: shrinks unknown7c by 32 down
// to 0, with unknownC9 set while it shrinks, and puts the effect on its
// side's player. Returns 1 (0x44ab30).
int EffectUpdate44ab30(Effect *effect)
{
    if (effect->unknown7c > 0.0f)
    {
        effect->unknownC9 = 1;
        effect->unknown7c -= 32.0f;
    }
    else
    {
        effect->unknownC9 = 0;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}

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

// An init callback of the effect table (type 32): allocates 0x294 bytes of
// vertices and sets the effect up as a ring of 32 segments, radius 640,
// around its position, from the angle in velocity.x, as wide as
// velocity.z. Returns 0, or -1 if the allocation fails (0x44aff0).
int EffectInit44aff0(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryManager.Alloc(0x294, "Effect");
    if (effect->vertices == NULL)
    {
        return -1;
    }
    effect->unknown48 = effect->pos;
    effect->unknown54.x = 0.0f;
    effect->unknown54.y = 0.0f;
    effect->unknown54.z = 1.0f;
    effect->unknown60.x = 0.0f;
    effect->unknown60.y = -1.0f;
    effect->unknown60.z = 0.0f;
    effect->unknown80 = effect->velocity.x;
    effect->unknown88 = effect->velocity.z;
    effect->unknown94 = 0.0f;
    effect->unknown98 = 0.0f;
    effect->unknown9c = 1.0f;
    effect->unknown8c = 0x20;
    effect->unknown7c = 640.0f;
    effect->unknownC9 = 1;
    return 0;
}

// An update callback of the effect table: grows unknown7c by 16 up to
// 96, sets unknownC9 and puts the effect on its side's player. Returns
// 1 (0x44b1d0).
int EffectUpdate44b1d0(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 96.0f)
    {
        effect->unknown7c += 16.0f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}
