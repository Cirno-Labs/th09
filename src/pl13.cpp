// Character 13's extra attacks and callbacks (pl/pl13.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// The unknown30450 of character 13: while unknown34 reaches (28 -
// unknownB4) * 5, sends an extra attack of type 21 from `pos` and
// takes as much off it. Returns 0 (0x44ba80).
int PlayerComboFunc44ba80(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (28 - g_GameManager.unknownB4) * 5)
    {
        g_GameManager.exAttackManager->AddAttack(21, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (28 - g_GameManager.unknownB4) * 5;
    }
    return 0;
}

// The draw callback of effect type 33: EffectDraw4414a0's fan
// (0x44baf0).
int EffectDraw44baf0(Effect *effect)
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

// The update callback of effect type 36: grows unknown7c by 0.5 up to 24,
// sets unknownC9 and puts the effect on its side's player. Returns 1
// (0x44bc30).
int EffectUpdate44bc30(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 24.0f)
    {
        effect->unknown7c += 0.5f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}
