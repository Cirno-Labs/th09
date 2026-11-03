// Character 8's extra attacks and callbacks (pl/pl08.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// The update callback of effect type 28: grows unknown7c by 8 up to 320,
// sets unknownC9, puts the effect on its side's player and clears
// unknown80. Returns 1 (0x4475c0).
int UpdateEffectType28(Effect *effect)
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

// The unknown30404 of character 8: whether `pos` is within the radius
// of the player's unknown368 of a point sqrt(3)/2 radii from the player, at a
// right angle to the effect's angle unknown80 and on the side away from
// `pos` (0x447620).
int CheckFocusAreaCharacter8(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = *pos - player->pos;
    D3DXVECTOR3 center;
    float angle = AddNormalizeAngle(player->unknown368->unknown80, 1.5707964f);
    D3DXVECTOR3 offset;

    SetPolar((Vector2 *)&offset, angle, player->unknown368->unknown7c * 0.8660254f);
    if (diff.x * offset.x + diff.y * offset.y > 0.0f)
    {
        center = player->pos - offset;
    }
    else
    {
        center = player->pos + offset;
    }
    diff = center - *pos;
    if (diff.x * diff.x + diff.y * diff.y < player->unknown368->unknown7c * player->unknown368->unknown7c)
    {
        return 1;
    }
    return 0;
}

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
int InitExAttackType16(ExAttack *attack)
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

// The update callback of extra attack types 16 and 23: flies in along
// the curve for 90 frames, then appears on the target's side drifting
// away from its middle and, after 20 frames, falls as a hit circle,
// turning back past the edges, until it touches the player there or
// leaves the bottom. Returns 1 once it is done (0x447890).
int UpdateExAttackTypes16And23(ExAttack *attack)
{
    ExAttackData16 *data = (ExAttackData16 *)attack->data;
    D3DXVECTOR3 size(24.0f, 24.0f, 0.0f);

    switch (data->state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            data->state++;
            attack->pos = data->targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
            if (attack->pos.x < 0.0f)
            {
                g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 0xb);
                data->unknown10 = -0.03f;
            }
            else
            {
                g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 0xc);
                data->unknown10 = 0.03f;
            }
            data->velocity.y = 2.0f;
            data->velocity.x = 0.0f;
        }
        break;
    case 1:
        if (attack->timer > 0x14)
        {
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
                return 0;
            }
            attack->pos += data->velocity;
            data->velocity.x += data->unknown10;
            if (attack->pos.x < -144.0f || attack->pos.x > 144.0f)
            {
                data->velocity.x *= -1.0f;
            }
            if (attack->pos.y >= 464.0f)
            {
                return 1;
            }
            g_GameManager.sides[attack->targetSide].player->unknown36c.AddCircle(&attack->pos, 13.0f, NULL);
        }
        break;
    case 2:
        if (attack->timer > 0x14)
        {
            return 1;
        }
        break;
    }
    return 0;
}

// The combo func of character 8: while unknown34 reaches 100 -
// unknownB4 * 3, sends an extra attack of type 16 from `pos` and takes
// as much off it. Returns 0 (0x447a80).
int SendComboAttacksCharacter8(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 100 - g_GameManager.rank.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(16, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 100 - g_GameManager.rank.unknownB4 * 3;
    }
    return 0;
}

// The init callback of extra attack type 23: type 16's, after 90
// frames, landing where it is on the target's side. Returns 0
// (0x447af0).
int InitExAttackType23(ExAttack *attack)
{
    ExAttackData16 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData16));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData16 *)attack->data;
    attack->timer = 0x5a;
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
    data->targetPos.x = attack->pos.x;
    data->targetPos.y = attack->pos.y;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
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

// The draw callback of effect type 28: DrawEffectType26's fan with two
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

// The unknown30404 of character 8: whether `pos` is within the radius of
// the player's unknown368 or, within 24 more, in reach of one of six
// points 24 beyond it, a sixth of a turn apart from unknown80 (0x447fb0).
int PlayerPosFunc447fb0(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 v = player->pos - *pos;
    float dist = v.x * v.x + v.y * v.y;
    float angle;

    if (dist < player->unknown368->unknown7c * player->unknown368->unknown7c)
    {
        return 1;
    }
    if (dist > (player->unknown368->unknown7c + 24.0f) * (player->unknown368->unknown7c + 24.0f))
    {
        return 0;
    }
    SetPolar((Vector2 *)&v, player->unknown368->unknown80, player->unknown368->unknown7c + 24.0f);
    v += player->pos;
    v = v - *pos;
    if (v.x * v.x + v.y * v.y < 576.0f)
    {
        return 1;
    }
    angle = AddNormalizeAngle(player->unknown368->unknown80, 1.0471976f);
    SetPolar((Vector2 *)&v, angle, player->unknown368->unknown7c + 24.0f);
    v += player->pos;
    v = v - *pos;
    if (v.x * v.x + v.y * v.y < 576.0f)
    {
        return 1;
    }
    angle = AddNormalizeAngle(player->unknown368->unknown80, 2.0943952f);
    SetPolar((Vector2 *)&v, angle, player->unknown368->unknown7c + 24.0f);
    v += player->pos;
    v = v - *pos;
    if (v.x * v.x + v.y * v.y < 576.0f)
    {
        return 1;
    }
    angle = AddNormalizeAngle(player->unknown368->unknown80, 3.1415927f);
    SetPolar((Vector2 *)&v, angle, player->unknown368->unknown7c + 24.0f);
    v += player->pos;
    v = v - *pos;
    if (v.x * v.x + v.y * v.y < 576.0f)
    {
        return 1;
    }
    angle = AddNormalizeAngle(player->unknown368->unknown80, 4.1887903f);
    SetPolar((Vector2 *)&v, angle, player->unknown368->unknown7c + 24.0f);
    v += player->pos;
    v = v - *pos;
    if (v.x * v.x + v.y * v.y < 576.0f)
    {
        return 1;
    }
    angle = AddNormalizeAngle(player->unknown368->unknown80, 5.2359877f);
    SetPolar((Vector2 *)&v, angle, player->unknown368->unknown7c + 24.0f);
    v += player->pos;
    v = v - *pos;
    if (v.x * v.x + v.y * v.y < 576.0f)
    {
        return 1;
    }
    return 0;
}
