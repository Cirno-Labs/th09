// Character 15's extra attacks and callbacks (pl/pl15.cpp).

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "unsorted.h"

// The update callback of effect type 37: grows unknown7c by 0.5 up to 8,
// sets unknownC9 and puts the effect on its side's player. Returns 1
// (0x44c290).
int UpdateEffectType37(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 8.0f)
    {
        effect->unknown7c += 0.5f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}

// The unknown30404 of character 15: whether `pos` is within the radius
// of the player's unknown368 across or up or down (0x44c2e0).
int CheckFocusAreaCharacter15(Player *player, D3DXVECTOR3 *pos)
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
int InitExAttackType26(ExAttack *attack)
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

// The update callback of extra attack type 26: flies in along the curve
// for 90 frames, then appears on the target's side and, after 20
// frames, fires a speeding bullet every 3 frames until it is 50 frames
// there. Returns 1 once it is done (0x44c4c0).
int UpdateExAttackType26(ExAttack *attack)
{
    ExAttackData00 *data = (ExAttackData00 *)attack->data;

    switch (data->state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 8);
            data->state++;
            attack->pos = data->targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
        }
        break;
    case 1:
        if (attack->timer > 0x14)
        {
            if (attack->timer.HasTickedEvery(3))
            {
                EnemyShooter shooter;

                shooter.pos = attack->pos;
                shooter.sprite = 0x13;
                shooter.aimMode = 2;
                shooter.count1 = 1;
                shooter.count2 = 1;
                shooter.color = 2;
                shooter.angle1 = 0.0f;
                shooter.speed1 = 0.2f;
                shooter.SetAccelCommand(0, 0, 120, 0.006666667f, -999.0f);
                shooter.angle2 = 0.0f;
                shooter.speed2 = 0.0f;
                shooter.unknown1fa = 0;
                shooter.flags = 0x10;
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
            }
            if (attack->timer > 0x32)
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

// The combo func of character 15: while unknown34 reaches 80 -
// unknownB4 * 3, sends an extra attack of type 26 from `pos` and takes
// as much off it. Returns 0 (0x44c680).
int SendComboAttacksCharacter15(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 80 - g_GameManager.rank.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(26, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 80 - g_GameManager.rank.unknownB4 * 3;
    }
    return 0;
}

// The init callback of effect type 37: InitEffectTypes10And18And24And25And26And28And33's ring with
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

// The draw callback of effect type 37: while unknownC9 is set, a
// vertical and then a horizontal beam through the effect, each a strip
// 2 * unknown7c wide across the playfield, brighter at the edges than
// in the middle. Returns 1 (0x44c790).
int EffectDraw44c790(Effect *effect)
{
    if (effect->unknownC9)
    {
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;
        VertexDiffuseXyzrwh *strip;

        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x - effect->unknown7c);
        vtx->pos.y = g_GameManager.ArenaToScreenY(0.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x - effect->unknown7c);
        vtx->pos.y = g_GameManager.ArenaToScreenY(448.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(0.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x30600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(448.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x30600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x + effect->unknown7c);
        vtx->pos.y = g_GameManager.ArenaToScreenY(0.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x + effect->unknown7c);
        vtx->pos.y = g_GameManager.ArenaToScreenY(448.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        g_SpriteEngine->DrawColoredStrip(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, 6);
        strip = vtx;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y - effect->unknown7c);
        vtx->pos.x = g_GameManager.ArenaToScreenX(-144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y - effect->unknown7c);
        vtx->pos.x = g_GameManager.ArenaToScreenX(144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->pos.x = g_GameManager.ArenaToScreenX(-144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x30600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->pos.x = g_GameManager.ArenaToScreenX(144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x30600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y + effect->unknown7c);
        vtx->pos.x = g_GameManager.ArenaToScreenX(-144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y + effect->unknown7c);
        vtx->pos.x = g_GameManager.ArenaToScreenX(144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        g_SpriteEngine->DrawColoredStrip(effect->sprites, strip, 6);
    }
    return 1;
}
