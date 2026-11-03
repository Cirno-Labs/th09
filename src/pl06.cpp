// Character 6's extra attacks and callbacks (pl/pl06.cpp).

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// The update callback of effect type 27: grows unknown7c by 0.5 up to 16,
// sets unknownC9 and puts the effect on its side's player. Returns 1
// (0x4454e0).
int UpdateEffectType27(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 16.0f)
    {
        effect->unknown7c += 0.5f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}

// A fire callback of the shot table (4): on the frame of `sd`, starts
// effect 0x23 at the player in reserved slot 5. Returns 0 (0x445530).
int FirePlayerShotType4(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame == sd->frame)
    {
        player->side->effectManager->SpawnReservedEffect(0x23, &player->pos, 5, 0xffffffff);
    }
    return 0;
}

// The unknown30404 of character 6: whether `pos` is within the radius
// of the player's unknown368 across (0x445580).
int CheckFocusAreaCharacter6(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    float radius = player->unknown368->unknown7c;

    if (diff.x * diff.x < radius * radius)
    {
        return 1;
    }
    return 0;
}

// The init callback of extra attack type 15: type 0's, landing between
// 128 and 328 down the other side (0x4455d0).
int InitExAttackType15(ExAttack *attack)
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
    data->targetPos.y = g_Rng.GetRandomF32InRange(200.0f) + 128.0f;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}

// The update callback of extra attack type 15: flies in along the curve
// for 90 frames, then appears on the target's side and, after 20
// frames, fires 8 speeding bullets at a random angle, again in color 4
// with aim mode 3, and is done. Returns 1 once it is done (0x445740).
int UpdateExAttackType15(ExAttack *attack)
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
            EnemyShooter shooter;

            shooter.pos = attack->pos;
            shooter.sprite = 0x13;
            shooter.aimMode = 5;
            shooter.count1 = 8;
            shooter.count2 = 1;
            shooter.color = 0;
            shooter.angle1 = g_Rng.GetRandomF32SignedInRange(3.1415927f);
            shooter.speed1 = 0.2f;
            shooter.SetAccelCommand(0, 0, 120, 0.016666668f, -999.0f);
            shooter.angle2 = 0.0f;
            shooter.speed2 = 0.0f;
            shooter.unknown1fa = 0;
            shooter.flags = 0x10;
            g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
            shooter.aimMode = 3;
            shooter.color = 4;
            shooter.SetAccelCommand(0, 0, 100, 0.016f, -999.0f);
            g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
            return 1;
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

// The combo func of character 6: while unknown34 reaches 100 -
// unknownB4 * 3, sends an extra attack of type 15 from `pos` and takes
// (28 - unknownB4) * 5 off it. Returns 0 (0x445930).
int SendComboAttacksCharacter6(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 100 - g_GameManager.rank.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(15, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (28 - g_GameManager.rank.unknownB4) * 5;
    }
    return 0;
}

// The init callback of effect types 27 and 36: EffectInit447c40's ring
// with 0x78 bytes of vertices and a radius of 0. Returns 0, or -1 if
// the allocation fails (0x4459a0).
int InitEffectTypes27And36(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryManager.Alloc(0x78, "Effect");
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

// The draw callback of effect type 27: a strip down the play area, from
// unknown7c left of the effect to unknown7c right of it, faint in the
// middle, if unknownC9 is set. Returns 1 (0x445a40).
int EffectDraw445a40(Effect *effect)
{
    if (effect->unknownC9)
    {
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;

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
        vtx->pos.x = g_GameManager.ArenaToScreenX(effect->unknown7c + vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(0.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(effect->unknown7c + vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(448.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
    }
    g_SpriteEngine->DrawColoredStrip(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, 6);
    return 1;
}

// The init callback of effect type 0x23: starts script 7 of the player's
// ANM on the first sprite, without writing depth and with no offset.
// Returns 0 (0x445c30).
int EffectInit445c30(Effect *effect)
{
    effect->mgr->side->player->anm->StartScript(effect->sprites, 7);
    effect->sprites->zWriteDisabled = 1;
    effect->sprites->posOffset.x = 0.0f;
    effect->sprites->posOffset.y = 0.0f;
    effect->sprites->posOffset.z = 0.0f;
    return 0;
}

// The update callback of effect type 0x23: fires the player's shots of
// table 2 on the effect's frame. Returns 1 (0x445c80).
int EffectUpdate445c80(Effect *effect)
{
    FireShots(effect->mgr->side->player, effect->unknownAc.AsFrames(), 2);
    return 1;
}

// A fire callback of the shot table (5): on the frame of `sd`, fires it
// unless the effect in reserved slot 5 is running. Returns 1 if it fired
// (0x445cb0).
int ShotFire445cb0(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (!player->side->effectManager->GetReservedEffect(5)->active && frame == sd->frame)
    {
        FireShot(player, bullet, sd);
        return 1;
    }
    return 0;
}

// A fire callback of the shot table (6): on the frame of `sd`, fires it
// from the effect in reserved slot 5, straight up at its speed. Returns
// 1 if it fired (0x445d00).
int ShotFire445d00(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame == sd->frame)
    {
        FireShot(player, bullet, sd);
        bullet->pos = player->side->effectManager->GetReservedEffect(5)->pos;
        bullet->angle = -1.5707964f;
        bullet->velocity.x = 0.0f;
        bullet->velocity.y = -sd->speed;
        return 1;
    }
    return 0;
}
