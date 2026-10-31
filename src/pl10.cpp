// Character 10's extra attacks and callbacks (pl/pl10.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "unsorted.h"

// The unknown30450 of character 10: while unknown34 reaches (25 -
// unknownB4) * 4, sends an extra attack of type 18 from `pos` and
// takes as much off it. Returns 0 (0x449780).
int PlayerComboFunc449780(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (25 - g_GameManager.unknownB4) * 4)
    {
        g_GameManager.exAttackManager->AddAttack(18, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (25 - g_GameManager.unknownB4) * 4;
    }
    return 0;
}

// An update callback of the effect table: grows unknown7c by 6 up to
// 128 and eases unknown80 towards -2pi/3 or -pi/3 by the way the player
// moves across, else towards -pi/2; sets unknownC9 and puts the effect
// on its side's player. Returns 1 (0x449ba0).
int EffectUpdate449ba0(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 128.0f)
    {
        effect->unknown7c += 6.0f;
    }
    if (g_GameManager.sides[effect->mgr->sideIdx].player->unknown1ccc < 0.0f)
    {
        effect->unknown80 += (-2.0943952f - effect->unknown80) * 0.02f;
    }
    else if (g_GameManager.sides[effect->mgr->sideIdx].player->unknown1ccc > 0.0f)
    {
        effect->unknown80 += (-1.0471976f - effect->unknown80) * 0.02f;
    }
    else
    {
        effect->unknown80 += (-1.5707964f - effect->unknown80) * 0.05f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}

// An update callback of the shot table (15): turns the bullet by pi/40
// and slows it down by 0.96 each frame. Returns 0 (0x449c70).
int ShotUpdate449c70(Player *player, PlayerBullet *bullet)
{
    float angle;
    float speed;

    bullet->angle = angle = AddNormalizeAngle(bullet->angle, 0.07853982f);
    bullet->speed = speed = bullet->speed * 0.96f;
    bullet->velocity.x = cosf(angle) * speed;
    bullet->velocity.y = sinf(angle) * speed;
    return 0;
}

// An update callback of the shot table (16): ShotUpdate449c70 turning
// the other way. Returns 0 (0x449ce0).
int ShotUpdate449ce0(Player *player, PlayerBullet *bullet)
{
    float angle;
    float speed;

    bullet->angle = angle = AddNormalizeAngle(bullet->angle, -0.07853982f);
    bullet->speed = speed = bullet->speed * 0.96f;
    bullet->velocity.x = cosf(angle) * speed;
    bullet->velocity.y = sinf(angle) * speed;
    return 0;
}
