// Character 5's extra attacks and callbacks (pl/pl05.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

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
int ExAttackInit444f00(ExAttack *attack)
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
int ExAttackInit445050(ExAttack *attack)
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

// The unknown30450 of character 5: while unknown34 reaches 25 -
// unknownB4 / 2, sends an extra attack of type 5 from `pos` and takes
// as much off it. Returns 0 (0x445330).
int PlayerComboFunc445330(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 25 - g_GameManager.unknownB4 / 2)
    {
        g_GameManager.exAttackManager->AddAttack(5, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 25 - g_GameManager.unknownB4 / 2;
    }
    return 0;
}

// The draw callback of effect type 24: EffectDraw4414a0's fan
// (0x4453a0).
int EffectDraw4453a0(Effect *effect)
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

// The update callback of effect type 27: grows unknown7c by 0.5 up to 16,
// sets unknownC9 and puts the effect on its side's player. Returns 1
// (0x4454e0).
int EffectUpdate4454e0(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 16.0f)
    {
        effect->unknown7c += 0.5f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}
