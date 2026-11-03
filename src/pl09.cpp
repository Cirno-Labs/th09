// Character 9's extra attacks and callbacks (pl/pl09.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// The data of extra attack type 17 (0x50 bytes): ExAttackData00 with
// the sprite variant it picks once it lands.
struct ExAttackData17
{
    int state;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    D3DXVECTOR3 tangent1;
    D3DXVECTOR3 tangent0;
    D3DXVECTOR3 targetPos;
    short variant;
};
ASSERT_SIZE(ExAttackData17, 0x50);

// The init callback of extra attack type 17, character 9's: type 0's,
// falling straight down at 0.5 to 2.5 once it lands. Returns 0
// (0x448320).
int InitExAttackType17(ExAttack *attack)
{
    ExAttackData17 *data;

    attack->AllocSprites(1, sizeof(ExAttackData17));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData17 *)attack->data;
    g_Rng.GetRandomF32InRange(-3.1415927f);
    data->state = 0;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    data->targetPos.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 8.0f);
    data->targetPos.y = g_Rng.GetRandomF32InRange(128.0f);
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    data->velocity.x = 0.0f;
    data->velocity.y = g_Rng.GetRandomF32InRange(2.0f) + 0.5f;
    data->velocity.z = 0.0f;
    return 0;
}

// The update callback of extra attack type 17: flies in along the curve
// for 90 frames, then appears on the target's side as one of four
// sprites and, after 20 frames, falls as a hit circle, bigger for the
// first two, until it touches the player there or leaves the bottom.
// Returns 1 once it is done (0x448490).
int UpdateExAttackType17(ExAttack *attack)
{
    ExAttackData17 *data = (ExAttackData17 *)attack->data;
    D3DXVECTOR3 size(24.0f, 24.0f, 0.0f);

    switch (data->state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            data->variant = g_Rng.GetRandomU16InRange(4);
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites,
                                                                               data->variant + 9);
            data->state++;
            attack->pos = data->targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
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
            if (data->variant < 2)
            {
                g_GameManager.sides[attack->targetSide].player->unknown36c.AddCircle(&attack->pos, 28.0f, NULL);
            }
            else
            {
                g_GameManager.sides[attack->targetSide].player->unknown36c.AddCircle(&attack->pos, 18.0f, NULL);
            }
            if (attack->pos.y >= 512.0f)
            {
                return 1;
            }
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

// The combo func of character 9: while unknown34 reaches (30 -
// unknownB4) * 2, sends an extra attack of type 17 from `pos` and
// takes as much off it. Returns 0 (0x448640).
int SendComboAttacksCharacter9(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (30 - g_GameManager.rank.unknownB4) * 2)
    {
        g_GameManager.exAttackManager->AddAttack(17, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (30 - g_GameManager.rank.unknownB4) * 2;
    }
    return 0;
}

// An init callback of the effect table: allocates 0xf78 bytes of
// vertices and sets the effect up as a ring of 32 segments, 32 out,
// around its position, as wide as velocity.z. Returns 0, or -1 if the
// allocation fails (0x4486b0).
int InitEffectType29(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryManager.Alloc(0xf78, "Effect");
    if (effect->vertices == NULL)
    {
        return -1;
    }
    effect->unknown48 = effect->pos;
    effect->unknown54.z = 1.0f;
    effect->unknown9c = 1.0f;
    effect->unknown54.x = 0.0f;
    effect->unknown54.y = 0.0f;
    effect->unknown60.x = 0.0f;
    effect->unknown60.y = -1.0f;
    effect->unknown60.z = 0.0f;
    effect->unknown80 = 0.0f;
    effect->unknown88 = effect->velocity.z;
    effect->unknown94 = 0.0f;
    effect->unknown98 = 0.0f;
    effect->unknown8c = 0x20;
    effect->unknown7c = 32.0f;
    effect->unknownC9 = 1;
    return 0;
}

// The draw callback of effect type 29: while unknownC9 is set, a ring
// fan of unknown8c segments at radius unknown7c about the effect, and
// six fans of half as many at radius 24 around it, a sixth of a turn
// apart from unknown80. Returns 1 (0x448750).
int DrawEffectType29(Effect *effect)
{
    float angle;

    if (effect->unknownC9)
    {
        int n = effect->unknown8c;
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;
        VertexDiffuseXyzrwh *fan = vtx;
        float step = 6.2831855f / (n - 1);
        int i;

        angle = -3.1415927f;
        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->w = 1.0f;
        vtx->diffuse = 0x40600040;
        vtx++;
        for (i = 0; i < n; i++)
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
            angle += step;
        }
        g_SpriteEngine->DrawColoredFan(effect->sprites, fan, n + 1);
        n /= 2;
        {
            float step = 6.2831855f / (n - 1);
            angle = effect->unknown80;
            D3DXVECTOR3 centre(0.0f, 0.0f, 0.0f);
            fan = vtx;
            SetPolar((Vector2 *)&centre, angle, effect->unknown7c + 24.0f);
            centre += effect->pos;
            vtx->pos = centre;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->w = 1.0f;
            vtx->diffuse = 0x40600040;
            vtx++;
            for (i = 0; i < n; i++)
            {
                if (angle >= 3.1415927f)
                {
                    angle -= 6.2831855f;
                }
                vtx->pos.z = 0.0f;
                SetPolar((Vector2 *)&vtx->pos, angle, 24.0f);
                vtx->pos += centre;
                vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
                vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
                vtx->w = 1.0f;
                vtx->diffuse = 0x70600040;
                vtx++;
                angle += step;
            }
            g_SpriteEngine->DrawColoredFan(effect->sprites, fan, n + 1);
        }
        {
            float step = 6.2831855f / (n - 1);
            angle = effect->unknown80;
            D3DXVECTOR3 centre(0.0f, 0.0f, 0.0f);
            angle = AddNormalizeAngle(angle, 1.0471976f);
            fan = vtx;
            SetPolar((Vector2 *)&centre, angle, effect->unknown7c + 24.0f);
            centre += effect->pos;
            vtx->pos = centre;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->w = 1.0f;
            vtx->diffuse = 0x40600040;
            vtx++;
            for (i = 0; i < n; i++)
            {
                if (angle >= 3.1415927f)
                {
                    angle -= 6.2831855f;
                }
                vtx->pos.z = 0.0f;
                SetPolar((Vector2 *)&vtx->pos, angle, 24.0f);
                vtx->pos += centre;
                vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
                vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
                vtx->w = 1.0f;
                vtx->diffuse = 0x70600040;
                vtx++;
                angle += step;
            }
            g_SpriteEngine->DrawColoredFan(effect->sprites, fan, n + 1);
        }
        {
            float step = 6.2831855f / (n - 1);
            angle = effect->unknown80;
            D3DXVECTOR3 centre(0.0f, 0.0f, 0.0f);
            angle = AddNormalizeAngle(angle, 2.0943952f);
            fan = vtx;
            SetPolar((Vector2 *)&centre, angle, effect->unknown7c + 24.0f);
            centre += effect->pos;
            vtx->pos = centre;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->w = 1.0f;
            vtx->diffuse = 0x40600040;
            vtx++;
            for (i = 0; i < n; i++)
            {
                if (angle >= 3.1415927f)
                {
                    angle -= 6.2831855f;
                }
                vtx->pos.z = 0.0f;
                SetPolar((Vector2 *)&vtx->pos, angle, 24.0f);
                vtx->pos += centre;
                vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
                vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
                vtx->w = 1.0f;
                vtx->diffuse = 0x70600040;
                vtx++;
                angle += step;
            }
            g_SpriteEngine->DrawColoredFan(effect->sprites, fan, n + 1);
        }
        {
            float step = 6.2831855f / (n - 1);
            angle = effect->unknown80;
            D3DXVECTOR3 centre(0.0f, 0.0f, 0.0f);
            angle = AddNormalizeAngle(angle, 3.1415927f);
            fan = vtx;
            SetPolar((Vector2 *)&centre, angle, effect->unknown7c + 24.0f);
            centre += effect->pos;
            vtx->pos = centre;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->w = 1.0f;
            vtx->diffuse = 0x40600040;
            vtx++;
            for (i = 0; i < n; i++)
            {
                if (angle >= 3.1415927f)
                {
                    angle -= 6.2831855f;
                }
                vtx->pos.z = 0.0f;
                SetPolar((Vector2 *)&vtx->pos, angle, 24.0f);
                vtx->pos += centre;
                vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
                vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
                vtx->w = 1.0f;
                vtx->diffuse = 0x70600040;
                vtx++;
                angle += step;
            }
            g_SpriteEngine->DrawColoredFan(effect->sprites, fan, n + 1);
        }
        {
            float step = 6.2831855f / (n - 1);
            angle = effect->unknown80;
            D3DXVECTOR3 centre(0.0f, 0.0f, 0.0f);
            angle = AddNormalizeAngle(angle, 4.1887903f);
            fan = vtx;
            SetPolar((Vector2 *)&centre, angle, effect->unknown7c + 24.0f);
            centre += effect->pos;
            vtx->pos = centre;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->w = 1.0f;
            vtx->diffuse = 0x40600040;
            vtx++;
            for (i = 0; i < n; i++)
            {
                if (angle >= 3.1415927f)
                {
                    angle -= 6.2831855f;
                }
                vtx->pos.z = 0.0f;
                SetPolar((Vector2 *)&vtx->pos, angle, 24.0f);
                vtx->pos += centre;
                vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
                vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
                vtx->w = 1.0f;
                vtx->diffuse = 0x70600040;
                vtx++;
                angle += step;
            }
            g_SpriteEngine->DrawColoredFan(effect->sprites, fan, n + 1);
        }
        {
            float step = 6.2831855f / (n - 1);
            angle = effect->unknown80;
            D3DXVECTOR3 centre(0.0f, 0.0f, 0.0f);
            angle = AddNormalizeAngle(angle, 5.2359877f);
            fan = vtx;
            SetPolar((Vector2 *)&centre, angle, effect->unknown7c + 24.0f);
            centre += effect->pos;
            vtx->pos = centre;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->w = 1.0f;
            vtx->diffuse = 0x40600040;
            vtx++;
            for (i = 0; i < n; i++)
            {
                if (angle >= 3.1415927f)
                {
                    angle -= 6.2831855f;
                }
                vtx->pos.z = 0.0f;
                SetPolar((Vector2 *)&vtx->pos, angle, 24.0f);
                vtx->pos += centre;
                vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
                vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
                vtx->w = 1.0f;
                vtx->diffuse = 0x70600040;
                vtx++;
                angle += step;
            }
            g_SpriteEngine->DrawColoredFan(effect->sprites, fan, n + 1);
        }
    }
    return 1;
}
