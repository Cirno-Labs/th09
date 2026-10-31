// Character 9's extra attacks and callbacks (pl/pl09.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
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
int ExAttackInit448320(ExAttack *attack)
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

// The unknown30450 of character 9: while unknown34 reaches (30 -
// unknownB4) * 2, sends an extra attack of type 17 from `pos` and
// takes as much off it. Returns 0 (0x448640).
int PlayerComboFunc448640(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (30 - g_GameManager.unknownB4) * 2)
    {
        g_GameManager.exAttackManager->AddAttack(17, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (30 - g_GameManager.unknownB4) * 2;
    }
    return 0;
}

// An init callback of the effect table: allocates 0xf78 bytes of
// vertices and sets the effect up as a ring of 32 segments, 32 out,
// around its position, as wide as velocity.z. Returns 0, or -1 if the
// allocation fails (0x4486b0).
int EffectInit4486b0(Effect *effect)
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

// An update callback of the effect table: grows unknown84 by pi/100 up
// to pi/2, sets unknownC9, puts the effect on its side's player and sets
// unknown80 to -pi/2. Returns 1 (0x449000).
int EffectUpdate449000(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown84 < 1.5707964f)
    {
        effect->unknown84 += 0.031415928f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    effect->unknown80 = -1.5707964f;
    return 1;
}

// The init callback of extra attack type 18: type 0's with four sprites
// and 0x158 bytes of data, ExAttackData00 and more (0x449070).
int ExAttackInit449070(ExAttack *attack)
{
    ExAttackData00 *data;
    float angle;

    attack->AllocSprites(4, 0x158);
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
    data->targetPos.y = g_Rng.GetRandomF32InRange(128.0f);
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}
