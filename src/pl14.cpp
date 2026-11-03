// Character 14's extra attacks and callbacks (pl/pl14.cpp).

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// The update callback of effect type 36: grows unknown7c by 0.5 up to 24,
// sets unknownC9 and puts the effect on its side's player. Returns 1
// (0x44bc30).
int UpdateEffectType36(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 24.0f)
    {
        effect->unknown7c += 0.5f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}

// The unknown30404 of character 14: whether `pos` is within the radius
// of the player's unknown368 up or down (0x44bc80).
int PlayerPosFunc44bc80(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    float radius = player->unknown368->unknown7c;

    if (diff.y * diff.y < radius * radius)
    {
        return 1;
    }
    return 0;
}

// The init callback of extra attack type 25: type 0's, landing between
// 32 and 160 down the other side (0x44bcd0).
int ExAttackInit44bcd0(ExAttack *attack)
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
    data->targetPos.y = g_Rng.GetRandomF32InRange(128.0f) + 32.0f;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}

// The update callback of extra attack type 25: flies in along the curve
// for 90 frames, then appears on the target's side and, after 20
// frames, fires a turning bullet down every 3 frames until it is 60
// frames there. Returns 1 once it is done (0x44be40).
int ExAttackUpdate44be40(ExAttack *attack)
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
                shooter.pos.y += 24.0f;
                shooter.sprite = 0x13;
                shooter.aimMode = 3;
                shooter.count1 = 1;
                shooter.count2 = 1;
                shooter.color = 1;
                shooter.angle1 = 1.5707964f;
                shooter.speed1 = 1.2f;
                shooter.SetAngleAccelCommand(0, 0, 120, g_Rng.GetRandomF32SignedInRange(0.015707964f),
                                             0.010833333f);
                shooter.angle2 = 0.0f;
                shooter.speed2 = 0.0f;
                shooter.unknown1fa = 0;
                shooter.flags = 0x20;
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
            }
            if (attack->timer > 0x3c)
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

// The combo func of character 14: while unknown34 reaches 100 -
// unknownB4 * 3, sends an extra attack of type 25 from `pos` and takes
// (28 - unknownB4) * 5 off it. Returns 0 (0x44c020).
int PlayerComboFunc44c020(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 100 - g_GameManager.rank.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(25, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (28 - g_GameManager.rank.unknownB4) * 5;
    }
    return 0;
}

// The draw callback of effect type 36: a strip across the play area,
// from unknown7c above the effect to unknown7c below it, faint in the
// middle, if unknownC9 is set. Returns 1 (0x44c090).
int EffectDraw44c090(Effect *effect)
{
    if (effect->unknownC9)
    {
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;

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
        vtx->pos.y = g_GameManager.ArenaToScreenY(effect->unknown7c + vtx->pos.y);
        vtx->pos.x = g_GameManager.ArenaToScreenX(-144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
        vtx++;
        vtx->pos = effect->pos;
        vtx->pos.y = g_GameManager.ArenaToScreenY(effect->unknown7c + vtx->pos.y);
        vtx->pos.x = g_GameManager.ArenaToScreenX(144.0f);
        vtx->w = 1.0f;
        vtx->diffuse = 0x70600040;
    }
    g_SpriteEngine->DrawColoredStrip(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, 6);
    return 1;
}
