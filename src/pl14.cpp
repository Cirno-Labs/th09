// Character 14's extra attacks and callbacks (pl/pl14.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"

// The unknown30404 of character 14: whether `pos` is within the radius
// of the player's unknown368 up or down (0x44bc80).
int PlayerPosFunc44bc80(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    float radius = player->unknown368->unknown7c;

    if (diff.y * diff.y < radius * radius)
    {
        return 1;
    }
    return 0;
}

// The unknown30450 of character 14: while unknown34 reaches 100 -
// unknownB4 * 3, sends an extra attack of type 25 from `pos` and takes
// (28 - unknownB4) * 5 off it. Returns 0 (0x44c020).
int PlayerComboFunc44c020(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 100 - g_GameManager.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(25, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (28 - g_GameManager.unknownB4) * 5;
    }
    return 0;
}

// The draw callback of effect type 36: a strip across the play area,
// from unknown7c above the effect to unknown7c below it, faint in the
// middle, if unknownC9 is set. Returns 1 (0x44c090).
int EffectDraw44c090(Effect *effect)
{
    if (effect->unknownC9)
    {
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;

        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y - effect->unknown7c);
        vtx->pos.x = g_GameManager.ArenaToScreenX(-144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y - effect->unknown7c);
        vtx->pos.x = g_GameManager.ArenaToScreenX(144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->pos.x = g_GameManager.ArenaToScreenX(-144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x30600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->pos.x = g_GameManager.ArenaToScreenX(144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x30600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(effect->unknown7c + vtx->pos.y);
        vtx->pos.x = g_GameManager.ArenaToScreenX(-144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(effect->unknown7c + vtx->pos.y);
        vtx->pos.x = g_GameManager.ArenaToScreenX(144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
    }
    g_SpriteEngine->DrawColoredStrip(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, 6);
    return 1;
}

// The update callback of effect type 37: grows unknown7c by 0.5 up to 8,
// sets unknownC9 and puts the effect on its side's player. Returns 1
// (0x44c290).
int EffectUpdate44c290(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 8.0f)
    {
        effect->unknown7c += 0.5f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}
