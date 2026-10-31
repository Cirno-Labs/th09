// Character 7's extra attacks and callbacks (pl/pl07.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

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

// The update callback of effect type 25: grows unknown7c by 4 up to 56,
// sets unknownC9 and puts the effect on its side's player, raised by 1.5
// times unknown7c. Returns 1 (0x445e00).
int EffectUpdate445e00(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 56.0f)
    {
        effect->unknown7c += 4.0f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    effect->pos.y -= effect->unknown7c * 1.5f;
    return 1;
}

// The unknown30404 of character 7: whether `pos` is within the radius of
// the player's unknown368 about a point 1.5 radii above the player
// (0x445e60).
int PlayerPosFunc445e60(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    Effect *circle = player->unknown368;
    float dy = diff.y - circle->unknown7c * 1.5f;

    if (diff.x * diff.x + dy * dy < circle->unknown7c * circle->unknown7c)
    {
        return 1;
    }
    return 0;
}

// The draw callback of effect type 25: EffectDraw4414a0's fan
// (0x447480).
int EffectDraw447480(Effect *effect)
{
    if (effect->unknownC9)
    {
        float delta = 6.2831855f / (effect->unknown8c - 1);
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;
        float angle = -3.1415927f;
        int i;

        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->w = 1.0f;
        vtx->diffuse = 0x40600040;
        vtx++;
        for (i = effect->unknown8c; i > 0; i--)
        {
            if (angle >= 3.1415927f)
            {
                angle -= 6.2831855f;
            }
            vtx->pos.z = 0.0f;
            SetPolar((Vector2 *)&vtx->pos, angle, effect->unknown7c);
            vtx->pos += effect->pos;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->w = 1.0f;
            vtx->diffuse = 0x70600040;
            vtx++;
            angle += delta;
        }
    }
    g_SpriteEngine->DrawColoredFan(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, effect->unknown8c + 1);
    return 1;
}

// The update callback of effect type 28: grows unknown7c by 8 up to 320,
// sets unknownC9, puts the effect on its side's player and clears
// unknown80. Returns 1 (0x4475c0).
int EffectUpdate4475c0(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 320.0f)
    {
        effect->unknown7c += 8.0f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    effect->unknown80 = 0.0f;
    return 1;
}

// The draw callback of effect type 28: EffectDraw444700's fan with two
// 60-degree arcs centred sqrt(3) / 2 unknown7c either side of the
// effect (0x447ce0).
int EffectDraw447ce0(Effect *effect)
{
    if (effect->unknownC9)
    {
        float delta = 1.0471976f / (effect->unknown8c / 2 - 1);
        float angle = AddNormalizeAngle(effect->unknown80, 1.0471976f);
        float across = AddNormalizeAngle(effect->unknown80, -1.5707964f);
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;
        D3DXVECTOR3 offset;
        int i;

        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->w = 1.0f;
        vtx->diffuse = 0x40600040;
        vtx++;
        SetPolar((Vector2 *)&offset, across, effect->unknown7c * 0.8660254f);
        for (i = effect->unknown8c; i > 0; i -= 2)
        {
            if (angle >= 3.1415927f)
            {
                angle -= 6.2831855f;
            }
            SetPolar((Vector2 *)&vtx->pos, angle, effect->unknown7c);
            vtx->pos += effect->pos + offset;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->pos.z = 0.0f;
            vtx->w = 1.0f;
            vtx->diffuse = 0x70600040;
            vtx++;
            angle += delta;
        }
        angle = AddNormalizeAngle(angle, 2.0943952f - delta);
        for (i = effect->unknown8c; i > 0; i -= 2)
        {
            if (angle >= 3.1415927f)
            {
                angle -= 6.2831855f;
            }
            SetPolar((Vector2 *)&vtx->pos, angle, effect->unknown7c);
            vtx->pos += effect->pos - offset;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->pos.z = 0.0f;
            vtx->w = 1.0f;
            vtx->diffuse = 0x70600040;
            vtx++;
            angle += delta;
        }
    }
    g_SpriteEngine->DrawColoredFan(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, effect->unknown8c + 1);
    return 1;
}
