// Character 5's extra attacks and callbacks (pl/pl05.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "pl.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// An update callback of the effect table: grows unk_7c by 8 up to
// 48, sets unk_c9 and puts the effect on its side's player. Returns
// 1 (0x444eb0).
int UpdateEffectType24(Effect *effect)
{
    effect->unk_c9 = 1;
    if (effect->unk_7c < 48.0f)
    {
        effect->unk_7c += 8.0f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}

// The data of extra attack types 5 and 10 (0x58 bytes): ExAttackData00
// with an acceleration after the velocity.
struct ExAttackData05
{
    int state;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 acceleration;
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    D3DXVECTOR3 tangent1;
    D3DXVECTOR3 tangent0;
    D3DXVECTOR3 targetPos;
};
ASSERT_SIZE(ExAttackData05, 0x58);

// The init callback of extra attack type 5: one sprite, flying in to
// near the target's player at the top of its side, then falling.
// Returns 0 (0x444f00).
int InitExAttackType5(ExAttack *attack)
{
    ExAttackData05 *data;

    attack->AllocSprites(1, sizeof(ExAttackData05));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData05 *)attack->data;
    data->velocity.x = 0.0f;
    data->velocity.y = 0.0f;
    data->velocity.z = 0.0f;
    data->acceleration.x = 0.0f;
    data->acceleration.y = 0.028f;
    data->acceleration.z = 0.0f;
    data->state = 0;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    data->targetPos.x =
        g_Rng.GetRandomF32SignedInRange(32.0f) + g_GameManager.sides[attack->targetSide].player->pos.x;
    data->targetPos.y = 16.0f;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}

// The init callback of extra attack type 10: type 5's, waiting 40
// frames and falling where it is on the target's side, drifting a
// little sideways. Returns 0 (0x445050).
int InitExAttackType10(ExAttack *attack)
{
    ExAttackData05 *data;

    attack->AllocSprites(1, sizeof(ExAttackData05));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData05 *)attack->data;
    data->velocity.x = 0.0f;
    data->velocity.y = 0.0f;
    data->velocity.z = 0.0f;
    data->acceleration.x = g_Rng.GetRandomF32SignedInRange(0.001f);
    data->acceleration.y = 0.025f;
    data->acceleration.z = 0.0f;
    data->state = 0;
    attack->timer = 40;
    attack->layer = 2;
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->targetPos.x = attack->pos.x;
    data->targetPos.y = attack->pos.y;
    data->targetPos.z = 0.0f;
    data->end.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->end.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}

// The update callback of extra attack types 5 and 10: flies in along
// the curve for 40 frames, then appears on the target's side and, after
// 12 frames, falls as a hit box, its fall capped at 4.5, until it
// touches the player there or leaves the bottom. Returns 1 once it is
// done (0x445190).
int UpdateExAttackTypes5And10(ExAttack *attack)
{
    ExAttackData05 *data = (ExAttackData05 *)attack->data;
    D3DXVECTOR3 size(6.0f, 32.0f, 0.0f);

    switch (data->state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 40.0f);
        if (attack->timer > 40)
        {
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 9);
            data->state++;
            attack->pos = data->targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
        }
        break;
    case 1:
        if (attack->timer > 12)
        {
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
                return 0;
            }
            attack->pos += data->velocity;
            data->velocity += data->acceleration;
            if (data->velocity.y > 4.5f)
            {
                data->velocity.y = 4.5f;
            }
            g_GameManager.sides[attack->targetSide].player->unk_36c.Add(&attack->pos, &size, NULL);
            if (attack->pos.y >= 480.0f)
            {
                return 1;
            }
        }
        break;
    case 2:
        if (attack->timer > 12)
        {
            return 1;
        }
        break;
    }
    return 0;
}

// The combo func of character 5: while unk_34 reaches 25 -
// unk_b4 / 2, sends an extra attack of type 5 from `pos` and takes
// as much off it. Returns 0 (0x445330).
int SendComboAttacksCharacter5(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unk_34 >= 25 - g_GameManager.rank.unk_b4 / 2)
    {
        g_GameManager.exAttackManager->AddAttack(5, pos, combo->player->sideIdx, NULL);
        combo->unk_34 -= 25 - g_GameManager.rank.unk_b4 / 2;
    }
    return 0;
}

// The draw callback of effect type 24: DrawEffectType10's fan
// (0x4453a0).
int DrawEffectType24(Effect *effect)
{
    if (effect->unk_c9)
    {
        float delta = 6.2831855f / (effect->unk_8c - 1);
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;
        float angle = -3.1415927f;
        int i;

        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->w = 1.0f;
        vtx->diffuse = 0x40600040;
        vtx++;
        for (i = effect->unk_8c; i > 0; i--)
        {
            if (angle >= 3.1415927f)
            {
                angle -= 6.2831855f;
            }
            vtx->pos.z = 0.0f;
            SetPolar((Vector2 *)&vtx->pos, angle, effect->unk_7c);
            vtx->pos += effect->pos;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->w = 1.0f;
            vtx->diffuse = 0x70600040;
            vtx++;
            angle += delta;
        }
    }
    g_SpriteEngine->DrawColoredFan(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, effect->unk_8c + 1);
    return 1;
}
