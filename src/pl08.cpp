// Character 8's extra attacks and callbacks (pl/pl08.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// The data of extra attack types 16 and 23 (0x50 bytes): ExAttackData00
// with a float after the velocity.
struct ExAttackData16
{
    int state;
    D3DXVECTOR3 velocity;
    float unknown10;
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    D3DXVECTOR3 tangent1;
    D3DXVECTOR3 tangent0;
    D3DXVECTOR3 targetPos;
};
ASSERT_SIZE(ExAttackData16, 0x50);

// The init callback of extra attack type 16: type 0's, landing further
// from the side's edges (0x447720).
int ExAttackInit447720(ExAttack *attack)
{
    ExAttackData16 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData16));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData16 *)attack->data;
    angle = g_Rng.GetRandomF32InRange(-3.1415927f);
    data->velocity.z = 0.0f;
    SetPolar((Vector2 *)&data->velocity, angle, 2.0f);
    data->state = 0;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    data->targetPos.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 64.0f);
    data->targetPos.y = g_Rng.GetRandomF32InRange(128.0f);
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}

// The unknown30450 of character 8: while unknown34 reaches 100 -
// unknownB4 * 3, sends an extra attack of type 16 from `pos` and takes
// as much off it. Returns 0 (0x447a80).
int PlayerComboFunc447a80(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 100 - g_GameManager.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(16, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 100 - g_GameManager.unknownB4 * 3;
    }
    return 0;
}

// An init callback of the effect table of types 10, 18, 24, 25, 26, 28 and 33: allocates 0x294 bytes of
// vertices and sets the effect up as a ring of 32 segments, radius 16,
// around its position, from the angle in velocity.x, as wide as
// velocity.z. Returns 0, or -1 if the allocation fails (0x447c40).
int EffectInit447c40(Effect *effect)
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
    effect->unknown7c = 16.0f;
    effect->unknownC9 = 1;
    return 0;
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

// The update callback of effect type 29: grows unknown7c by 4 up to 48,
// turns unknown80 by pi/60, sets unknownC9 and puts the effect on its
// side's player. Returns 1 (0x447f40).
int EffectUpdate447f40(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 48.0f)
    {
        effect->unknown7c += 4.0f;
    }
    effect->unknown80 = AddNormalizeAngle(effect->unknown80, 0.05235988f);
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}
