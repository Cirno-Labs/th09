// Character 7's extra attacks and callbacks (pl/pl07.cpp).

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// The combo func of character 7: while unknown34 reaches 200 -
// unknownB4 * 3, sends an extra attack of type 6 from `pos` and takes
// as much off it and 5 off unknown30, down to 0. Returns 0 (0x445d80).
int SendComboAttacksCharacter7(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 200 - g_GameManager.rank.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(6, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 200 - g_GameManager.rank.unknownB4 * 3;
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
int UpdateEffectType25(Effect *effect)
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
int CheckFocusAreaCharacter7(Player *player, D3DXVECTOR3 *pos)
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

// The data of extra attack types 6 to 9, 11 and 12 (0x5c bytes):
// ExAttackData00 with the heading it flies at and the angle its bullets
// turn through.
struct ExAttackData08
{
    int state;
    // Toggled after each ring, for types 11 and 12.
    int alternate;
    float heading;
    float spin;
    // How much the heading turns each frame, for type 6.
    float turn;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    D3DXVECTOR3 tangent1;
    D3DXVECTOR3 tangent0;
    D3DXVECTOR3 targetPos;
};
ASSERT_SIZE(ExAttackData08, 0x5c);

// The init callback of extra attack type 6: type 8's, flying in from
// the left or the right of the target's play area, at a random height,
// heading across it within 30 degrees, turning by up to 3/4 degree a
// frame. Returns 0 (0x445ec0).
int InitExAttackType6(ExAttack *attack)
{
    ExAttackData08 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData08));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData08 *)attack->data;
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
    if (g_Rng.GetRandomU32InRange(2))
    {
        data->targetPos.x = -144.0f;
        data->heading = g_Rng.GetRandomF32SignedInRange(0.5235988f);
    data->turn = g_Rng.GetRandomF32SignedInRange(0.01308997f);
    }
    else
    {
        data->targetPos.x = 144.0f;
        data->heading = AddNormalizeAngle(3.1415927f, g_Rng.GetRandomF32SignedInRange(0.5235988f));
    data->turn = g_Rng.GetRandomF32SignedInRange(0.01308997f);
    }
    data->targetPos.y = g_Rng.GetRandomF32InRange(320.0f);
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}

// The update callback of extra attack type 6: type 8's, turning as it
// goes and firing two bullets along its heading every 20 - unknownB4 / 8
// frames. Returns 1 once it is done (0x446060).
int UpdateExAttackType6(ExAttack *attack)
{
    ExAttackData08 *data = (ExAttackData08 *)attack->data;
    D3DXVECTOR3 size(24.0f, 24.0f, 0.0f);

    switch (data->state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 9);
            data->state++;
            attack->pos = data->targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
        }
        break;
    case 1:
        if (attack->timer > 0x14)
        {
            if (attack->timer.HasTickedEvery(20 - g_GameManager.rank.unknownB4 / 8))
            {
                EnemyShooter shooter;

                shooter.pos = attack->pos;
                shooter.sprite = 6;
                shooter.aimMode = 5;
                shooter.count1 = 2;
                shooter.count2 = 1;
                shooter.color = 6;
                shooter.angle1 = data->heading;
                shooter.speed1 = 0.0f;
                shooter.SetWaitCommand(0, 0, 0x50);
                shooter.SetAccelCommand(1, 0, 120, g_GameManager.rank.unknownB4 * 0.00037878787f + 0.015833333f,
                                        -999.0f);
                shooter.angle2 = 0.0f;
                shooter.speed2 = 0.0f;
                shooter.unknown1fa = 0;
                shooter.flags = 0x20014;
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
            }
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
            }
            else
            {
                SetPolar((Vector2 *)&data->velocity, data->heading, 1.5f);
                data->heading = AddNormalizeAngle(data->heading, data->turn);
                attack->pos += data->velocity;
                if (attack->pos.y >= 480.0f || attack->pos.y <= -32.0f || attack->pos.x >= 176.0f ||
                    attack->pos.x <= -176.0f)
                {
                    return 1;
                }
            }
        }
        attack->angle = data->heading;
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

// The init callback of extra attack type 7: type 9's, landing at
// either side of the target's side, lower, heading inwards (0x446320).
int InitExAttackType7(ExAttack *attack)
{
    ExAttackData08 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData08));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData08 *)attack->data;
    angle = g_Rng.GetRandomF32InRange(-3.1415927f);
    data->velocity.z = 0.0f;
    SetPolar((Vector2 *)&data->velocity, angle, 2.0f);
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
    if (g_Rng.GetRandomU32InRange(2))
    {
        data->targetPos.x = -144.0f;
        data->heading = 0.0f;
    }
    else
    {
        data->targetPos.x = 144.0f;
        data->heading = 3.1415927f;
    }
    data->targetPos.y = 224.0f;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    data->spin = g_Rng.GetRandomF32SignedInRange(3.1415927f);
    return 0;
}

// The update callback of extra attack type 7: type 8's, firing each
// ring twice, the second mirrored, of bullet type 0 aimed with mode 3.
// Returns 1 once it is done (0x4464a0).
int UpdateExAttackType7(ExAttack *attack)
{
    ExAttackData08 *data = (ExAttackData08 *)attack->data;
    D3DXVECTOR3 size(24.0f, 24.0f, 0.0f);

    switch (data->state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 9);
            data->state++;
            attack->pos = data->targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
        }
        break;
    case 1:
        if (attack->timer > 0x14)
        {
            if (attack->timer.HasTickedEvery(10 - g_GameManager.difficulty))
            {
                float angle = data->spin = AddNormalizeAngle(data->spin, 0.10471976f);
                EnemyShooter shooter;

                shooter.pos = attack->pos;
                shooter.sprite = 6;
                shooter.aimMode = 5;
                shooter.count1 = g_GameManager.sides[1 - attack->targetSide].player->unknownA0 / 5 + 2;
                shooter.count2 = 1;
                shooter.color = 2;
                shooter.angle1 = angle;
                shooter.speed1 = 0.0f;
                shooter.SetWaitCommand(0, 0, 0x50);
                shooter.SetAccelCommand(
                    1, 0, 120,
                    (g_GameManager.sides[1 - attack->targetSide].player->unknownA0 * 0.1f + 2.0f) * 0.008333334f,
                    -999.0f);
                shooter.angle2 = 0.0f;
                shooter.speed2 = 0.0f;
                shooter.unknown1fa = 0;
                shooter.flags = 0x20014;
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
                shooter.sprite = 0;
                shooter.aimMode = 3;
                shooter.angle1 = AddNormalizeAngle(3.1415927f, -data->spin);
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
            }
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
            }
            else
            {
                SetPolar((Vector2 *)&data->velocity, data->heading, 1.5f);
                attack->pos += data->velocity;
                if (attack->pos.y >= 480.0f || attack->pos.y <= -32.0f || attack->pos.x >= 176.0f ||
                    attack->pos.x <= -176.0f)
                {
                    return 1;
                }
            }
        }
        attack->angle = data->heading;
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

// The init callback of extra attack type 9: type 0's, after 90 frames,
// landing at the left of the target's side heading right (0x4467c0).
int InitExAttackType9(ExAttack *attack)
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

// The update callback of extra attack types 8 and 9: flies in along the
// curve for 90 frames, then appears on the target's side and, after 20
// frames, crosses it at its heading as a hit box, firing a spinning
// ring of bullets every 10 - difficulty frames, until it touches the
// player there or leaves the side. Returns 1 once it is done
// (0x446920).
int UpdateExAttackTypes8And9(ExAttack *attack)
{
    ExAttackData08 *data = (ExAttackData08 *)attack->data;
    D3DXVECTOR3 size(24.0f, 24.0f, 0.0f);

    switch (data->state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 9);
            data->state++;
            attack->pos = data->targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
        }
        break;
    case 1:
        if (attack->timer > 0x14)
        {
            if (attack->timer.HasTickedEvery(10 - g_GameManager.difficulty))
            {
                float angle = data->spin = AddNormalizeAngle(data->spin, 0.10471976f);
                EnemyShooter shooter;

                shooter.pos = attack->pos;
                shooter.sprite = 6;
                shooter.aimMode = 5;
                shooter.count1 = g_GameManager.sides[1 - attack->targetSide].player->unknownA0 / 5 + 2;
                shooter.count2 = 1;
                shooter.color = 2;
                shooter.angle1 = angle;
                shooter.speed1 = 0.0f;
                shooter.SetWaitCommand(0, 0, 0x50);
                shooter.SetAccelCommand(
                    1, 0, 120,
                    (g_GameManager.sides[1 - attack->targetSide].player->unknownA0 * 0.1f + 2.0f) * 0.008333334f,
                    -999.0f);
                shooter.angle2 = 0.0f;
                shooter.speed2 = 0.0f;
                shooter.unknown1fa = 0;
                shooter.flags = 0x20014;
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
            }
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
            }
            else
            {
                SetPolar((Vector2 *)&data->velocity, data->heading, 1.5f);
                attack->pos += data->velocity;
                if (attack->pos.y >= 480.0f || attack->pos.y <= -32.0f || attack->pos.x >= 176.0f ||
                    attack->pos.x <= -176.0f)
                {
                    return 1;
                }
            }
        }
        attack->angle = data->heading;
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

// The init callback of extra attack type 8: type 9's, landing at the
// right heading left (0x446c10).
int InitExAttackType8(ExAttack *attack)
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
    data->targetPos.x = 144.0f;
    data->heading = 3.1415927f;
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

// The init callback of extra attack types 11 and 12: type 9's, heading
// at the angle AddAttack passed as pos.z, which it clears, and landing
// where it is on the target's side (0x446d80).
int InitExAttackTypes11And12(ExAttack *attack)
{
    ExAttackData08 *data;

    attack->AllocSprites(1, sizeof(ExAttackData08));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData08 *)attack->data;
    g_Rng.GetRandomF32InRange(-3.1415927f);
    data->velocity.z = 0.0f;
    SetPolar((Vector2 *)&data->velocity, attack->pos.z, 2.0f);
    data->heading = attack->pos.z;
    data->state = 0;
    attack->layer = 2;
    attack->timer = 0x5a;
    attack->pos.z = 0.0f;
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->targetPos.x = attack->pos.x;
    data->targetPos.y = attack->pos.y;
    data->targetPos.z = 0.0f;
    data->end.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->end.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    data->spin = g_Rng.GetRandomF32SignedInRange(3.1415927f);
    return 0;
}

// The update callback of extra attack type 11: type 8's, firing every
// 10 frames a single spinning bullet of alternating types, accelerated
// by the other side's player's unknownA4. Returns 1 once it is done
// (0x446ee0).
int UpdateExAttackType11(ExAttack *attack)
{
    ExAttackData08 *data = (ExAttackData08 *)attack->data;
    D3DXVECTOR3 size(24.0f, 24.0f, 0.0f);

    switch (data->state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 9);
            data->state++;
            attack->pos = data->targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
        }
        break;
    case 1:
        if (attack->timer > 0x14)
        {
            if (attack->timer.HasTickedEvery(10))
            {
                float angle = data->spin = AddNormalizeAngle(data->spin, 0.10471976f);
                EnemyShooter shooter;

                shooter.pos = attack->pos;
                shooter.sprite = data->alternate != 0 ? 6 : 0;
                shooter.aimMode = 5;
                shooter.count1 = 1;
                shooter.count2 = 1;
                shooter.color = 2;
                shooter.angle1 = angle;
                shooter.speed1 = 0.0f;
                shooter.SetWaitCommand(0, 0, 0x50);
                shooter.SetAccelCommand(
                    1, 0, 120,
                    (g_GameManager.sides[1 - attack->targetSide].player->unknownA4 * 0.05f + 2.0f) * 0.008333334f,
                    -999.0f);
                shooter.angle2 = 0.0f;
                shooter.speed2 = 0.0f;
                shooter.unknown1fa = 0;
                shooter.flags = 0x20014;
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
                data->alternate = 1 - data->alternate;
            }
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
            }
            else
            {
                SetPolar((Vector2 *)&data->velocity, data->heading, 1.5f);
                attack->pos += data->velocity;
                if (attack->pos.y >= 480.0f || attack->pos.y <= -32.0f || attack->pos.x >= 176.0f ||
                    attack->pos.x <= -176.0f)
                {
                    return 1;
                }
            }
        }
        attack->angle = data->heading;
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

// The update callback of extra attack type 12: type 11's, its bullets
// fired along the heading while the spin still turns. Returns 1 once it
// is done (0x4471b0).
int UpdateExAttackType12(ExAttack *attack)
{
    ExAttackData08 *data = (ExAttackData08 *)attack->data;
    D3DXVECTOR3 size(24.0f, 24.0f, 0.0f);

    switch (data->state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(attack->sprites, 9);
            data->state++;
            attack->pos = data->targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
        }
        break;
    case 1:
        if (attack->timer > 0x14)
        {
            if (attack->timer.HasTickedEvery(10))
            {
                data->spin = AddNormalizeAngle(data->spin, 0.10471976f);
                EnemyShooter shooter;

                shooter.pos = attack->pos;
                shooter.sprite = data->alternate != 0 ? 6 : 0;
                shooter.aimMode = 5;
                shooter.count1 = 1;
                shooter.count2 = 1;
                shooter.color = 2;
                shooter.angle1 = data->heading;
                shooter.speed1 = 0.0f;
                shooter.SetWaitCommand(0, 0, 0x50);
                shooter.SetAccelCommand(
                    1, 0, 120,
                    (g_GameManager.sides[1 - attack->targetSide].player->unknownA4 * 0.05f + 2.0f) * 0.008333334f,
                    -999.0f);
                shooter.angle2 = 0.0f;
                shooter.speed2 = 0.0f;
                shooter.unknown1fa = 0;
                shooter.flags = 0x20014;
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern(&shooter);
                data->alternate = 1 - data->alternate;
            }
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
            }
            else
            {
                SetPolar((Vector2 *)&data->velocity, data->heading, 1.5f);
                attack->pos += data->velocity;
                if (attack->pos.y >= 480.0f || attack->pos.y <= -32.0f || attack->pos.x >= 176.0f ||
                    attack->pos.x <= -176.0f)
                {
                    return 1;
                }
            }
        }
        attack->angle = data->heading;
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

// The draw callback of effect type 25: DrawEffectType10's fan
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
