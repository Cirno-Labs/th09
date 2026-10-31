// Character 10's extra attacks and callbacks (pl/pl10.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

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

// The combo func of character 10: while unknown34 reaches (25 -
// unknownB4) * 4, sends an extra attack of type 18 from `pos` and
// takes as much off it. Returns 0 (0x449780).
int PlayerComboFunc449780(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (25 - g_GameManager.rank.unknownB4) * 4)
    {
        g_GameManager.exAttackManager->AddAttack(18, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (25 - g_GameManager.rank.unknownB4) * 4;
    }
    return 0;
}

// The init callback of extra attack type 24: type 18's, after 90
// frames, landing where it is on the target's side. Returns 0
// (0x4497f0).
int ExAttackInit4497f0(ExAttack *attack)
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
    attack->timer = 0x5a;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    data->targetPos.x = attack->pos.x;
    data->targetPos.y = attack->pos.y;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}

// The init callback of effect type 30: EffectInit442f50's ring of
// radius 128, turned to -pi/2 after all. Returns 0, or -1 if the
// allocation fails (0x449940).
int EffectInit449940(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryManager.Alloc(0x294, "Effect");
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
    effect->unknown80 = AddNormalizeAngle(effect->mgr->side->player->unknown1cd8, 3.1415927f);
    effect->unknown94 = 0.0f;
    effect->unknown98 = 0.0f;
    effect->unknown84 = 0.0f;
    effect->unknown88 = effect->velocity.z;
    effect->unknown8c = 0x10;
    effect->unknown9c = 1.0f;
    effect->unknown7c = 128.0f;
    effect->unknownC9 = 1;
    effect->unknown80 = -1.5707964f;
    return 0;
}

// The draw callback of effect type 30: EffectDraw443010's sector
// (0x449a10).
int EffectDraw449a10(Effect *effect)
{
    if (effect->unknownC9)
    {
        float delta = effect->unknown84 / (effect->unknown8c - 1);
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;
        float angle = effect->unknown80 - effect->unknown84 * 0.5f;
        int i;

        vtx->pos = effect->pos;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->w = 1.0f;
        vtx->diffuse = 0x30600040;
        vtx++;
        for (i = effect->unknown8c; i > 0; i--)
        {
            vtx->pos.z = 0.0f;
            angle = AddNormalizeAngle(angle, 0.0f);
            SetPolar((Vector2 *)&vtx->pos, angle, effect->unknown7c);
            vtx->pos += effect->pos;
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->w = 1.0f;
            vtx->diffuse = 0x50600040;
            vtx++;
            angle += delta;
        }
    }
    g_SpriteEngine->DrawColoredFan(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, effect->unknown8c + 1);
    return 1;
}

// Which side of the line from a to b p is on: 1 to the left (a positive
// cross product), -1 to the right, 0 on it. Static, with p, a and b in
// edx, ecx and eax (0x449b50).
static int LineSide(D3DXVECTOR3 *p, D3DXVECTOR3 *a, D3DXVECTOR3 *b)
{
    float cross = (p->y - a->y) * b->x + (a->y - b->y) * p->x + (b->y - p->y) * a->x;

    if (cross > 0.0f)
    {
        return 1;
    }
    if (cross < 0.0f)
    {
        return -1;
    }
    return 0;
}

// An update callback of the effect table: grows unknown7c by 6 up to
// 128 and eases unknown80 towards -2pi/3 or -pi/3 by the way the player
// moves across, else towards -pi/2; sets unknownC9 and puts the effect
// on its side's player. Returns 1 (0x449ba0).
int EffectUpdate449ba0(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 128.0f)
    {
        effect->unknown7c += 6.0f;
    }
    if (g_GameManager.sides[effect->mgr->sideIdx].player->unknown1ccc.x < 0.0f)
    {
        effect->unknown80 += (-2.0943952f - effect->unknown80) * 0.02f;
    }
    else if (g_GameManager.sides[effect->mgr->sideIdx].player->unknown1ccc.x > 0.0f)
    {
        effect->unknown80 += (-1.0471976f - effect->unknown80) * 0.02f;
    }
    else
    {
        effect->unknown80 += (-1.5707964f - effect->unknown80) * 0.05f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}

// An update callback of the shot table (15): turns the bullet by pi/40
// and slows it down by 0.96 each frame. Returns 0 (0x449c70).
int ShotUpdate449c70(Player *player, PlayerBullet *bullet)
{
    float angle;
    float speed;

    bullet->angle = angle = AddNormalizeAngle(bullet->angle, 0.07853982f);
    bullet->speed = speed = bullet->speed * 0.96f;
    bullet->velocity.x = cosf(angle) * speed;
    bullet->velocity.y = sinf(angle) * speed;
    return 0;
}

// An update callback of the shot table (16): ShotUpdate449c70 turning
// the other way. Returns 0 (0x449ce0).
int ShotUpdate449ce0(Player *player, PlayerBullet *bullet)
{
    float angle;
    float speed;

    bullet->angle = angle = AddNormalizeAngle(bullet->angle, -0.07853982f);
    bullet->speed = speed = bullet->speed * 0.96f;
    bullet->velocity.x = cosf(angle) * speed;
    bullet->velocity.y = sinf(angle) * speed;
    return 0;
}

// The unknown30404 of character 10: whether `pos` is within the
// pentagram of the player's unknown368, its points unknown7c from the
// player starting at the angle unknown80, as three triangles from each
// point to the far inner corners (0x449d50).
int PlayerPosFunc449d50(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = *pos - player->pos;
    D3DXVECTOR3 pts[10];
    Effect *effect = player->unknown368;
    float angle = effect->unknown80;

    SetPolar((Vector2 *)&pts[0], angle, effect->unknown7c);
    angle = AddNormalizeAngle(angle, 1.2566371f);
    SetPolar((Vector2 *)&pts[2], angle, effect->unknown7c);
    angle = AddNormalizeAngle(angle, 1.2566371f);
    SetPolar((Vector2 *)&pts[4], angle, effect->unknown7c);
    angle = AddNormalizeAngle(angle, 1.2566371f);
    SetPolar((Vector2 *)&pts[6], angle, effect->unknown7c);
    angle = AddNormalizeAngle(angle, 1.2566371f);
    SetPolar((Vector2 *)&pts[8], angle, effect->unknown7c);
    pts[1] = (pts[0] - pts[4]) / 1.618f + pts[4];
    pts[3] = (pts[2] - pts[6]) / 1.618f + pts[6];
    pts[5] = (pts[4] - pts[8]) / 1.618f + pts[8];
    pts[7] = (pts[6] - pts[0]) / 1.618f + pts[0];
    pts[9] = (pts[8] - pts[2]) / 1.618f + pts[2];
    if (LineSide(&diff, &pts[0], &pts[4]) >= 0 && LineSide(&diff, &pts[4], &pts[7]) >= 0 &&
        LineSide(&diff, &pts[7], &pts[0]) >= 0)
    {
        return 1;
    }
    if (LineSide(&diff, &pts[8], &pts[2]) >= 0 && LineSide(&diff, &pts[2], &pts[5]) >= 0 &&
        LineSide(&diff, &pts[5], &pts[8]) >= 0)
    {
        return 1;
    }
    if (LineSide(&diff, &pts[6], &pts[0]) >= 0 && LineSide(&diff, &pts[0], &pts[3]) >= 0 &&
        LineSide(&diff, &pts[3], &pts[6]) >= 0)
    {
        return 1;
    }
    return 0;
}
