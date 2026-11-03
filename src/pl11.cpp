// Character 11's extra attacks and callbacks (pl/pl11.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// The init callback of extra attack type 19, character 11's: type 0's
// curve to a random spot on the lower half of the other side, and the
// disc there, 80 in radius give or take 8, each rim point with its own
// speed. Returns 0 (0x44a030).
int InitExAttackType19(ExAttack *attack)
{
    ExAttackData19 *data;
    VertexTex1DiffuseXyzrwh *vtx;
    D3DXVECTOR3 uv;
    float angle;
    float speed;
    int i;

    attack->AllocSprites(1, sizeof(ExAttackData19));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData19 *)attack->data;
    {
        float dir = g_Rng.GetRandomF32InRange(-3.1415927f);
        data->base.velocity.z = 0.0f;
        SetPolar((Vector2 *)&data->base.velocity, dir, 2.0f);
    }
    data->base.state = 0;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->base.tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->base.tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->base.start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->base.start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->base.start.z = 0.0f;
    data->base.targetPos.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 24.0f);
    data->base.targetPos.y = 448.0f - g_Rng.GetRandomF32InRange(256.0f);
    data->base.targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->base.end.x = g_GameManager.ArenaToScreenX(data->base.targetPos.x);
    data->base.end.y = g_GameManager.ArenaToScreenY(data->base.targetPos.y);
    data->base.end.z = 0.0f;
    g_App.SetCamera(attack->side);
    data->points[0] = data->base.targetPos;
    vtx = data->vertices;
    vtx->w = 1.0f;
    vtx->uv.x = 0.5f;
    vtx->uv.y = 0.5f;
    angle = -3.1415927f;
    vtx++;
    speed = g_Rng.GetRandomF32SignedInRange(0.06666667f);
    for (i = 1; i < 0x21; i++)
    {
        if (angle >= 3.1415927f)
        {
            angle -= 6.2831855f;
        }
        vtx->w = 1.0f;
        SetPolar((Vector2 *)&uv, angle, 0.5f);
        vtx->uv.x = uv.x + 0.5f;
        vtx->uv.y = uv.y + 0.5f;
        vtx++;
        data->points[i].z = 0.0f;
        data->radii[i] = g_Rng.GetRandomF32SignedInRange(8.0f) + 80.0f;
        data->radiusSpeeds[i] = speed;
        speed += g_Rng.GetRandomF32SignedInRange(0.033333335f);
        if (speed < -0.06666667f)
        {
            speed = -0.06666667f;
        }
        else if (speed > 0.06666667f)
        {
            speed = 0.06666667f;
        }
        SetPolar((Vector2 *)&data->points[i], angle, data->radii[i]);
        data->points[i] += data->base.targetPos;
        angle += 0.2026834f;
    }
    data->scrollU = g_Rng.GetRandomF32SignedInRange(0.008333334f);
    data->scrollV = g_Rng.GetRandomF32SignedInRange(0.008333334f);
    return 0;
}

// The update callback of extra attack type 19: flies in along the curve
// for 90 frames, then appears hidden on the target's side as the disc,
// its texture scrolling and its rim wobbling, slowing the player there
// by unknown1ce4 and unknown1ce8 while they are within 64 of it from
// frame 20 to 300. Returns 1 once its sprite has hidden (0x44a330).
int UpdateExAttackType19(ExAttack *attack)
{
    // Not matching yet: the angle and the vertex pointer swap stack
    // slots, and the scroll adds in the loop take their operands the
    // other way round.
    ExAttackData19 *data = (ExAttackData19 *)attack->data;
    int i;
    int j;

    switch (data->base.state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->base.end, &data->base.start, &data->base.tangent0,
                           &data->base.tangent1, attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            data->base.state++;
            attack->pos = data->base.targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 9);
            attack->sprites->visible = 0;
        }
        break;
    case 1:
    {
        VertexTex1DiffuseXyzrwh *vtx = data->vertices;
        float angle = -3.1415927f;

        vtx->uv.x += data->scrollU;
        if (vtx->uv.x < 0.0f)
        {
            for (j = 0; j < 0x21; j++)
            {
                data->vertices[j].uv.x += 1.0f;
            }
        }
        vtx->uv.y += data->scrollU;
        if (vtx->uv.y < 0.0f)
        {
            for (j = 0; j < 0x21; j++)
            {
                data->vertices[j].uv.y += 1.0f;
            }
        }
        vtx++;
        for (i = 1; i < 0x21; i++)
        {
            vtx->uv.x += data->scrollU;
            if (vtx->uv.x < 0.0f)
            {
                for (j = 0; j < 0x21; j++)
                {
                    data->vertices[j].uv.x += 1.0f;
                }
            }
            vtx->uv.y += data->scrollU;
            if (vtx->uv.y < 0.0f)
            {
                for (j = 0; j < 0x21; j++)
                {
                    data->vertices[j].uv.y += 1.0f;
                }
            }
            data->radii[i] += data->radiusSpeeds[i];
            SetPolar((Vector2 *)&data->points[i], angle, data->radii[i]);
            data->points[i] += data->base.targetPos;
            angle += 0.2026834f;
            vtx++;
        }
        if (attack->timer < 300 && attack->timer > 0x14)
        {
            D3DXVECTOR3 diff = g_GameManager.sides[attack->targetSide].player->pos - attack->pos;
            if (diff.x * diff.x + diff.y * diff.y <= 4096.0f)
            {
                g_GameManager.sides[attack->targetSide].player->unknown1ce4 *= 0.4f;
                g_GameManager.sides[attack->targetSide].player->unknown1ce8 *= 0.4f;
            }
        }
        if (!attack->sprites->shown)
        {
            return 1;
        }
        break;
    }
    case 2:
        if (attack->timer > 0x14)
        {
            return 1;
        }
        break;
    }
    return 0;
}

// The combo func of character 11: while unknown34 reaches (28 -
// unknownB4) * 5, sends an extra attack of type 19 from `pos` and
// takes as much off it. Returns 0 (0x44a670).
int SendComboAttacksCharacter11(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (28 - g_GameManager.rank.unknownB4) * 5)
    {
        g_GameManager.exAttackManager->AddAttack(19, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (28 - g_GameManager.rank.unknownB4) * 5;
    }
    return 0;
}

// The init callback of effect type 31: InitEffectTypes10And18And24And25And26And28And33's ring of
// radius 16, from -pi/2. Returns 0, or -1 if the allocation fails (0x44a6e0).
int InitEffectType31(Effect *effect)
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
    effect->unknown88 = effect->velocity.z;
    effect->unknown94 = 0.0f;
    effect->unknown98 = 0.0f;
    effect->unknown9c = 1.0f;
    effect->unknown8c = 0x20;
    effect->unknown7c = 16.0f;
    effect->unknownC9 = 1;
    effect->unknown80 = -1.5707964f;
    return 0;
}

// The draw callback of effect type 31: while unknownC9 is set, a
// pentagram of radius unknown7c turned by unknown80 around the effect,
// as a fan from its centre through the five points and the corners
// between them. Returns 1 (0x44a780).
int DrawEffectType31(Effect *effect)
{
    if (effect->unknownC9)
    {
        float angle = effect->unknown80;
        D3DXVECTOR3 points[10];
        VertexDiffuseXyzrwh *vtx;
        int i;

        SetPolar((Vector2 *)&points[0], angle, effect->unknown7c);
        angle = AddNormalizeAngle(angle, 1.2566371f);
        SetPolar((Vector2 *)&points[2], angle, effect->unknown7c);
        angle = AddNormalizeAngle(angle, 1.2566371f);
        SetPolar((Vector2 *)&points[4], angle, effect->unknown7c);
        angle = AddNormalizeAngle(angle, 1.2566371f);
        SetPolar((Vector2 *)&points[6], angle, effect->unknown7c);
        angle = AddNormalizeAngle(angle, 1.2566371f);
        SetPolar((Vector2 *)&points[8], angle, effect->unknown7c);
        points[1] = (points[0] - points[4]) / 1.618f + points[4];
        points[3] = (points[2] - points[6]) / 1.618f + points[6];
        points[5] = (points[4] - points[8]) / 1.618f + points[8];
        points[7] = (points[6] - points[0]) / 1.618f + points[0];
        points[9] = (points[8] - points[2]) / 1.618f + points[2];
        for (i = 0; i < 10; i++)
        {
            points[i].z = 0.0f;
        }
        vtx = (VertexDiffuseXyzrwh *)effect->vertices;
        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->w = 1.0f;
        vtx->diffuse = 0x20600040;
        vtx++;
        for (i = 0; i < 10; i++)
        {
            vtx->pos = points[i] + effect->pos;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->w = 1.0f;
            vtx->diffuse = 0x70600040;
            vtx++;
        }
        vtx->pos = points[0] + effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
    }
    g_SpriteEngine->DrawColoredFan(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, 0xc);
    return 1;
}

// Does nothing. Returns 0. Every empty callback of the extra attack and
// spell card type tables points here, the linker having folded them
// into this copy (0x44ab20).
int ReturnZero(ExAttack *attack)
{
    return 0;
}
