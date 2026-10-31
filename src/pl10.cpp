// Character 10's extra attacks and callbacks (pl/pl10.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
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

// The init callback of effect type 30: EffectInit442f50's ring of
// radius 128, turned to -pi/2 after all. Returns 0, or -1 if the
// allocation fails (0x449940).
int EffectInit449940(Effect *effect)
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
    effect->unknown80 = AddNormalizeAngle(effect->mgr->side->player->unknown1cd8, 3.1415927f);
    effect->unknown94 = 0.0f;
    effect->unknown98 = 0.0f;
    effect->unknown84 = 0.0f;
    effect->unknown88 = effect->velocity.z;
    effect->unknown8c = 0x10;
    effect->unknown9c = 1.0f;
    effect->unknown7c = 128.0f;
    effect->unknownC9 = 1;
    effect->unknown80 = -1.5707964f;
    return 0;
}

// The draw callback of effect type 30: EffectDraw443010's sector
// (0x449a10).
int EffectDraw449a10(Effect *effect)
{
    if (effect->unknownC9)
    {
        float delta = effect->unknown84 / (effect->unknown8c - 1);
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;
        float angle = effect->unknown80 - effect->unknown84 * 0.5f;
        int i;

        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->w = 1.0f;
        vtx->diffuse = 0x30600040;
        vtx++;
        for (i = effect->unknown8c; i > 0; i--)
        {
            vtx->pos.z = 0.0f;
            angle = AddNormalizeAngle(angle, 0.0f);
            SetPolar((Vector2 *)&vtx->pos, angle, effect->unknown7c);
            vtx->pos += effect->pos;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->w = 1.0f;
            vtx->diffuse = 0x50600040;
            vtx++;
            angle += delta;
        }
    }
    g_SpriteEngine->DrawColoredFan(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, effect->unknown8c + 1);
    return 1;
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
