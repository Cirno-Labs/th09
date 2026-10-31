// Character 7's extra attacks and callbacks (pl/pl07.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// The unknown30450 of character 7: while unknown34 reaches 200 -
// unknownB4 * 3, sends an extra attack of type 6 from `pos` and takes
// as much off it and 5 off unknown30, down to 0. Returns 0 (0x445d80).
int PlayerComboFunc445d80(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 200 - g_GameManager.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(6, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 200 - g_GameManager.unknownB4 * 3;
        combo->unknown30 -= 5;
        if (combo->unknown30 < 0)
        {
            combo->unknown30 = 0;
        }
    }
    return 0;
}

// The update callback of effect type 25: grows unknown7c by 4 up to 56,
// sets unknownC9 and puts the effect on its side's player, raised by 1.5
// times unknown7c. Returns 1 (0x445e00).
int EffectUpdate445e00(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 56.0f)
    {
        effect->unknown7c += 4.0f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    effect->pos.y -= effect->unknown7c * 1.5f;
    return 1;
}

// The unknown30404 of character 7: whether `pos` is within the radius of
// the player's unknown368 about a point 1.5 radii above the player
// (0x445e60).
int PlayerPosFunc445e60(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    Effect *circle = player->unknown368;
    float dy = diff.y - circle->unknown7c * 1.5f;

    if (diff.x * diff.x + dy * dy < circle->unknown7c * circle->unknown7c)
    {
        return 1;
    }
    return 0;
}

// The data of extra attack types 8 and 9 (0x5c bytes): ExAttackData00
// with the heading it flies at and the angle its bullets turn through.
struct ExAttackData08
{
    int state;
    int unknown04;
    float heading;
    float spin;
    int unknown10;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    D3DXVECTOR3 tangent1;
    D3DXVECTOR3 tangent0;
    D3DXVECTOR3 targetPos;
};
ASSERT_SIZE(ExAttackData08, 0x5c);

// The init callback of extra attack type 9: type 0's, after 90 frames,
// landing at the left of the target's side heading right (0x4467c0).
int ExAttackInit4467c0(ExAttack *attack)
{
    ExAttackData08 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData08));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData08 *)attack->data;
    angle = g_Rng.GetRandomF32InRange(-3.1415927f);
    data->velocity.z = 0.0f;
    SetPolar((Vector2 *)&data->velocity, angle, 2.0f);
    data->heading = angle;
    data->state = 0;
    attack->layer = 2;
    attack->timer = 0x5a;
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    data->targetPos.x = -144.0f;
    data->heading = 0.0f;
    data->targetPos.y = 192.0f;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    data->spin = g_Rng.GetRandomF32SignedInRange(3.1415927f);
    return 0;
}

// The draw callback of effect type 25: EffectDraw4414a0's fan
// (0x447480).
int EffectDraw447480(Effect *effect)
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

// The update callback of effect type 28: grows unknown7c by 8 up to 320,
// sets unknownC9, puts the effect on its side's player and clears
// unknown80. Returns 1 (0x4475c0).
int EffectUpdate4475c0(Effect *effect)
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
