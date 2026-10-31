// Character 15's extra attacks and callbacks (pl/pl15.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "unsorted.h"

// The unknown30404 of character 15: whether `pos` is within the radius
// of the player's unknown368 across or up or down (0x44c2e0).
int PlayerPosFunc44c2e0(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;

    if (diff.x * diff.x < player->unknown368->unknown7c * player->unknown368->unknown7c ||
        diff.y * diff.y < player->unknown368->unknown7c * player->unknown368->unknown7c)
    {
        return 1;
    }
    return 0;
}

// The init callback of extra attack type 26: type 0's, landing between
// 200 and 328 down the other side (0x44c350).
int ExAttackInit44c350(ExAttack *attack)
{
    ExAttackData00 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData00));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData00 *)attack->data;
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
    data->targetPos.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 8.0f);
    data->targetPos.y = g_Rng.GetRandomF32InRange(128.0f) + 200.0f;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}

// The unknown30450 of character 15: while unknown34 reaches 80 -
// unknownB4 * 3, sends an extra attack of type 26 from `pos` and takes
// as much off it. Returns 0 (0x44c680).
int PlayerComboFunc44c680(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 80 - g_GameManager.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(26, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 80 - g_GameManager.unknownB4 * 3;
    }
    return 0;
}

// The init callback of effect type 37: EffectInit447c40's ring with
// 0xf0 bytes of vertices and a radius of 0. Returns 0, or -1 if the
// allocation fails (0x44c6f0).
int EffectInit44c6f0(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryManager.Alloc(0xf0, "Effect");
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
    effect->unknown7c = 0.0f;
    effect->unknownC9 = 1;
    return 0;
}
