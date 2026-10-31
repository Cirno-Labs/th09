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

// The update callback of extra attack type 17: flies in along the curve
// for 90 frames, then appears on the target's side as one of four
// sprites and, after 20 frames, falls as a hit circle, bigger for the
// first two, until it touches the player there or leaves the bottom.
// Returns 1 once it is done (0x448490).
int ExAttackUpdate448490(ExAttack *attack)
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

// The data of extra attack type 18 (0x158 bytes): type 0's, then a
// heading and the trail of the last 16 positions and headings.
struct ExAttackData18
{
    ExAttackData00 base;
    float angle;
    float speed;
    // Set once it has bounced off a side.
    int bounced;
    D3DXVECTOR3 positions[16];
    float angles[16];
};
ASSERT_SIZE(ExAttackData18, 0x158);

// The init callback of extra attack type 18: type 0's with four sprites
// and ExAttackData18 (0x449070).
int ExAttackInit449070(ExAttack *attack)
{
    ExAttackData00 *data;
    float angle;

    attack->AllocSprites(4, sizeof(ExAttackData18));
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

// The update callback of extra attack type 18: flies in along the curve
// for 90 frames, then heads for the target's player, speeding up, with
// three sprites trailing on its 2nd, 4th and 6th last positions. Bounces
// off a side once, adds a circle of 12 to the player's unknown36c ahead
// of it and stops when it hits the player. Returns 1 once below 480
// (0x4491e0).
int ExAttackUpdate4491e0(ExAttack *attack)
{
    ExAttackData18 *data = (ExAttackData18 *)attack->data;
    D3DXVECTOR3 size(24.0f, 24.0f, 0.0f);
    int i;

    switch (data->base.state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->base.end, &data->base.start, &data->base.tangent0,
                           &data->base.tangent1, attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[0], 9);
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[1], 10);
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[2], 10);
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[3], 10);
            data->base.state++;
            attack->pos = data->base.targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
            data->angle = g_GameManager.sides[attack->targetSide].player->AngleToPlayer(&attack->pos);
            data->speed = 0.2f;
            data->bounced = 0;
            for (i = 0; i < 16; i++)
            {
                data->positions[i] = attack->pos;
                data->angles[i] = attack->angle;
            }
        }
        break;
    case 1:
        for (i = 15; i > 0; i--)
        {
            data->positions[i] = data->positions[i - 1];
            data->angles[i] = data->angles[i - 1];
        }
        data->positions[0] = attack->pos;
        data->angles[0] = data->angle;
        attack->sprites[0].SetRotationZ(data->angle);
        attack->sprites[1].posOffset = data->positions[1] - attack->pos;
        attack->sprites[1].SetRotationZ(data->angles[1]);
        attack->sprites[2].posOffset = data->positions[3] - attack->pos;
        attack->sprites[2].SetRotationZ(data->angles[3]);
        attack->sprites[3].posOffset = data->positions[5] - attack->pos;
        attack->sprites[3].SetRotationZ(data->angles[5]);
        if (attack->timer > 0x14)
        {
            D3DXVECTOR3 ahead;

            SetPolar((Vector2 *)&data->base.velocity, data->angle, data->speed);
            data->speed += 0.1f;
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->base.state = 2;
                attack->timer = 0;
                attack->sprites[0].SetPendingInterrupt(1);
                attack->sprites[1].SetPendingInterrupt(1);
                attack->sprites[2].SetPendingInterrupt(1);
                attack->sprites[3].SetPendingInterrupt(1);
                return 0;
            }
            attack->pos += data->base.velocity;
            if (!data->bounced)
            {
                if (attack->pos.x < -144.0f)
                {
                    data->base.velocity.x = -data->base.velocity.x;
                    data->angle = 3.1415927f - data->angle;
                    data->angle = AddNormalizeAngle(data->angle, 0.0f);
                    data->bounced = 1;
                }
                else if (attack->pos.x > 144.0f)
                {
                    data->base.velocity.x = -data->base.velocity.x;
                    data->angle = 3.1415927f - data->angle;
                    data->angle = AddNormalizeAngle(data->angle, 0.0f);
                    data->bounced = 1;
                }
            }
            SetPolar((Vector2 *)&ahead, data->angle, data->speed);
            ahead += attack->pos;
            g_GameManager.sides[attack->targetSide].player->unknown36c.AddCircle(&ahead, 12.0f, NULL);
            if (attack->pos.y >= 480.0f)
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
