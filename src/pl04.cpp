// Character 4's extra attacks and callbacks (pl/pl04.cpp).

#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// The state of effect type 34, which it keeps in its vertices (0xef8
// bytes).
struct EffectData34
{
    // 0 while growing, then 1.
    int state;
    unsigned char pad_4[0x50 - 0x4];
    float unknown50;
    unsigned char pad_54[0xce8 - 0x54];
    // Random offsets, 32 of each row's 33 refreshed each frame.
    float unknownCe8[4][0x21];
};
ASSERT_SIZE(EffectData34, 0xef8);

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

// The data of extra attack type 14: type 0's, then the fan and rings
// its draw fills (0xf48 bytes).
struct ExAttackData14
{
    ExAttackData00 base;
    // Cleared after every draw.
    int unknown4c;
    // The radius of the disc, from 4 up to 64 as it bursts.
    float radius;
    VertexDiffuseXyzrwh fan[0x21];
    VertexDiffuseXyzrwh rings[4][0x21];
    // Random offsets to the radius, per ring point.
    float offsets[4][0x21];
};
ASSERT_SIZE(ExAttackData14, 0xf48);

// The init callback of extra attack type 14, character 4's: type 0's,
// starting off at around pi/2, with a disc of radius 4. Returns 0
// (0x443c70).
int ExAttackInit443c70(ExAttack *attack)
{
    ExAttackData14 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData14));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData14 *)attack->data;
    angle = g_Rng.GetRandomF32InRange(0.19634955f) + 1.5707964f;
    data->base.velocity.z = 0.0f;
    SetPolar((Vector2 *)&data->base.velocity, angle, 2.0f);
    data->base.state = 0;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->base.tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->base.tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->base.start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->base.start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->base.start.z = 0.0f;
    data->base.targetPos.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 8.0f);
    data->base.targetPos.y = g_Rng.GetRandomF32InRange(128.0f);
    data->base.targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->base.end.x = g_GameManager.ArenaToScreenX(data->base.targetPos.x);
    data->base.end.y = g_GameManager.ArenaToScreenY(data->base.targetPos.y);
    data->base.end.z = 0.0f;
    g_App.SetCamera(attack->side);
    data->radius = 4.0f;
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

// The draw callback of effect type 26: a fan about the effect of two
// arcs of unknown7c in radius, 120 degrees each, centred unknown7c / 2
// either side of it across unknown80, if unknownC9 is set. Returns 1
// (0x444700).
int EffectDraw444700(Effect *effect)
{
    if (effect->unknownC9)
    {
        float delta = 2.0943952f / (effect->unknown8c / 2 - 1);
        float angle = AddNormalizeAngle(effect->unknown80, 0.5235988f);
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
        SetPolar((Vector2 *)&offset, across, effect->unknown7c * 0.5f);
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
        angle = AddNormalizeAngle(angle, 1.0471976f - delta);
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

// The init callback of effect type 34: allocates its zeroed state in
// vertices and sets it up as EffectInit447c40's ring of radius 16.
// Returns 0, or -1 if the allocation fails (0x444960).
int EffectInit444960(Effect *effect)
{
    EffectData34 *data;

    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryManager.Alloc(sizeof(EffectData34), "Effect");
    if (effect->vertices == NULL)
    {
        return -1;
    }
    memset(effect->vertices, 0, sizeof(EffectData34));
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
    data = (EffectData34 *)effect->vertices;
    data->unknown50 = 0.0f;
    data->state = 0;
    return 0;
}

// The update callback of effect type 34: over 50 frames, eases
// unknown50 out to 64; then for 30 frames fills unknownCe8 with random
// offsets as wide as the frames passed. Returns 0 once done, else 1
// (0x444a20).
int EffectUpdate444a20(Effect *effect)
{
    EffectData34 *data = (EffectData34 *)effect->vertices;
    float t;
    int i;
    int j;

    if (data->state == 0)
    {
        t = 1.0f - effect->unknownAc.AsFloat() * 0.02f;
        data->unknown50 = (1.0f - t * t) * 64.0f;
        if (effect->unknownAc >= 50)
        {
            data->state = 1;
            effect->unknownAc = 0;
        }
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            for (j = 0; j < 0x20; j++)
            {
                data->unknownCe8[i][j] = g_Rng.GetRandomF32SignedInRange(effect->unknownAc.AsFloat());
            }
        }
        if (effect->unknownAc >= 30)
        {
            return 0;
        }
    }
    return 1;
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
