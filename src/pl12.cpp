// Character 12's extra attacks and callbacks (pl/pl12.cpp).

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// The update callback of effect type 32: shrinks unknown7c by 32 down
// to 0, with unknownC9 set while it shrinks, and puts the effect on its
// side's player. Returns 1 (0x44ab30).
int EffectUpdate44ab30(Effect *effect)
{
    if (effect->unknown7c > 0.0f)
    {
        effect->unknownC9 = 1;
        effect->unknown7c -= 32.0f;
    }
    else
    {
        effect->unknownC9 = 0;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}

// A fire callback of the shot table (7): on the frame of `sd`, fires it
// at unknown30364 turned by its angle, or straight up once that is left
// of the side. Returns 1 if it fired (0x44ab90).
int ShotFire44ab90(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    float angle;

    if (frame == sd->frame)
    {
        angle = player->AngleFromPlayer(&player->unknown30364) + sd->angle;
        if (player->unknown30364.x < -144.0f)
        {
            angle = -1.5707964f;
        }
        FireShot(player, bullet, sd);
        bullet->velocity.x = cosf(angle) * sd->speed;
        bullet->velocity.y = sinf(angle) * sd->speed;
        bullet->angle = angle;
        return 1;
    }
    return 0;
}

// The data of extra attack type 20 (0x5c bytes): type 0's, then the
// spiral it fires.
struct ExAttackData20
{
    ExAttackData00 base;
    // The speed and angle of the next ring, and how far the angle turns
    // after each.
    float speed;
    float angle;
    float angleStep;
    // How many rings it has fired.
    int count;
};
ASSERT_SIZE(ExAttackData20, 0x5c);

// The init callback of extra attack type 20, character 12's: type 0's
// curve to a random spot near the top of the other side, with a spiral
// from a random angle turning pi/30 either way. Returns 0 (0x44ac20).
int ExAttackInit44ac20(ExAttack *attack)
{
    ExAttackData20 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData20));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData20 *)attack->data;
    angle = g_Rng.GetRandomF32InRange(-3.1415927f);
    data->base.velocity.z = 0.0f;
    SetPolar((Vector2 *)&data->base.velocity, angle, 2.0f);
    data->base.state = 0;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->base.tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->base.tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->base.start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->base.start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->base.start.z = 0.0f;
    data->base.targetPos.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 8.0f);
    data->base.targetPos.y = g_Rng.GetRandomF32InRange(144.0f) + 32.0f;
    data->base.targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->base.end.x = g_GameManager.ArenaToScreenX(data->base.targetPos.x);
    data->base.end.y = g_GameManager.ArenaToScreenY(data->base.targetPos.y);
    data->base.end.z = 0.0f;
    g_App.SetCamera(attack->side);
    data->speed = 1.0f;
    data->angle = g_Rng.GetRandomF32SignedInRange(3.1415927f);
    data->angleStep = g_Rng.GetRandomU16InRange(2) ? 0.10471976f : -0.10471976f;
    data->count = 0;
    return 0;
}

// The update callback of extra attack type 20: flies in along the curve
// for 90 frames, then appears hidden on the target's side and, every
// other frame for 30 frames, fires a ring of 8 bullets there, each
// faster and turned on by angleStep, in three colors in turn. Returns 1
// once it is done (0x44ade0).
int ExAttackUpdate44ade0(ExAttack *attack)
{
    ExAttackData20 *data = (ExAttackData20 *)attack->data;

    switch (data->base.state)
    {
    case 0:
        HermiteInterpolate(&attack->pos, &data->base.end, &data->base.start, &data->base.tangent0,
                           &data->base.tangent1, attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            data->base.state++;
            attack->pos = data->base.targetPos;
            attack->layer = attack->targetSide;
            attack->timer = 0;
            attack->sprites->visible = 0;
        }
        break;
    case 1:
        if (attack->timer < 30)
        {
            if (attack->timer.HasTickedEvery(2))
            {
                EnemyShooter shooter;

                shooter.pos = attack->pos;
                shooter.sprite = 0x15;
                shooter.aimMode = 5;
                shooter.count1 = 8;
                shooter.count2 = 1;
                shooter.color = data->count % 3;
                shooter.angle1 = data->angle;
                shooter.speed1 = data->speed;
                shooter.angle2 = 0.0f;
                shooter.speed2 = 0.0f;
                shooter.unknown1fa = 0;
                shooter.flags = 0;
                g_GameManager.sides[attack->targetSide].bulletManager->FirePattern2(&shooter);
                data->count++;
                data->angle += data->angleStep;
                data->speed += 0.2f;
            }
            break;
        }
        return 1;
    case 2:
        if (attack->timer > 0x14)
        {
            return 1;
        }
        break;
    }
    return 0;
}

// The unknown30450 of character 12: while unknown34 reaches (40 -
// unknownB4) * 4, sends an extra attack of type 20 from `pos` and
// takes as much off it. Returns 0 (0x44af80).
int PlayerComboFunc44af80(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (40 - g_GameManager.unknownB4) * 4)
    {
        g_GameManager.exAttackManager->AddAttack(20, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (40 - g_GameManager.unknownB4) * 4;
    }
    return 0;
}

// An init callback of the effect table (type 32): allocates 0x294 bytes of
// vertices and sets the effect up as a ring of 32 segments, radius 640,
// around its position, from the angle in velocity.x, as wide as
// velocity.z. Returns 0, or -1 if the allocation fails (0x44aff0).
int EffectInit44aff0(Effect *effect)
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
    effect->unknown80 = effect->velocity.x;
    effect->unknown88 = effect->velocity.z;
    effect->unknown94 = 0.0f;
    effect->unknown98 = 0.0f;
    effect->unknown9c = 1.0f;
    effect->unknown8c = 0x20;
    effect->unknown7c = 640.0f;
    effect->unknownC9 = 1;
    return 0;
}

// The draw callback of effect type 32: EffectDraw4414a0's fan, drawn only if unknownC9 is set
// (0x44b090).
int EffectDraw44b090(Effect *effect)
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
        g_SpriteEngine->DrawColoredFan(effect->sprites, (VertexDiffuseXyzrwh *)effect->vertices, effect->unknown8c + 1);
    }
    return 1;
}

// The update callback of effect type 33: grows unknown7c by 16 up to 96,
// sets unknownC9 and puts the effect on its side's player. Returns 1
// (0x44b1d0).
int EffectUpdate44b1d0(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 96.0f)
    {
        effect->unknown7c += 16.0f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}

// The data of extra attack type 21 (0x68c bytes): type 19's disc and
// an angle.
struct ExAttackData21
{
    ExAttackData19 disc;
    // -pi to begin with.
    float unknown688;
};
ASSERT_SIZE(ExAttackData21, 0x68c);

// The init callback of extra attack type 21: type 19's disc, at a
// random spot 128 to 256 down the other side, without a scrolling
// texture. Returns 0 (0x44b220).
int ExAttackInit44b220(ExAttack *attack)
{
    ExAttackData21 *data;
    VertexTex1DiffuseXyzrwh *vtx;
    D3DXVECTOR3 uv;
    float angle;
    float speed;
    int i;

    attack->AllocSprites(1, sizeof(ExAttackData21));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData21 *)attack->data;
    {
        float dir = g_Rng.GetRandomF32InRange(-3.1415927f);
        data->disc.base.velocity.z = 0.0f;
        SetPolar((Vector2 *)&data->disc.base.velocity, dir, 2.0f);
    }
    data->disc.base.state = 0;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->disc.base.tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->disc.base.tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->disc.base.start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->disc.base.start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->disc.base.start.z = 0.0f;
    data->disc.base.targetPos.x = g_Rng.GetRandomF32SignedInRange(g_GameManager.arena[2] * 0.5f - 24.0f);
    data->disc.base.targetPos.y = g_Rng.GetRandomF32InRange(128.0f) + 128.0f;
    data->disc.base.targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->disc.base.end.x = g_GameManager.ArenaToScreenX(data->disc.base.targetPos.x);
    data->disc.base.end.y = g_GameManager.ArenaToScreenY(data->disc.base.targetPos.y);
    data->disc.base.end.z = 0.0f;
    g_App.SetCamera(attack->side);
    data->disc.points[0] = data->disc.base.targetPos;
    data->unknown688 = -3.1415927f;
    vtx = data->disc.vertices;
    vtx->w = 1.0f;
    vtx->uv.x = 0.5f;
    vtx->uv.y = 0.5f;
    angle = -3.1415927f;
    vtx++;
    speed = g_Rng.GetRandomF32SignedInRange(0.06666667f);
    for (i = 1; i < 0x21; i++)
    {
        if (angle >= 3.1415927f)
        {
            angle -= 6.2831855f;
        }
        vtx->w = 1.0f;
        SetPolar((Vector2 *)&uv, angle, 0.5f);
        vtx->uv.x = uv.x + 0.5f;
        vtx->uv.y = uv.y + 0.5f;
        vtx++;
        data->disc.points[i].z = 0.0f;
        data->disc.radii[i] = g_Rng.GetRandomF32SignedInRange(8.0f) + 80.0f;
        data->disc.radiusSpeeds[i] = speed;
        speed += g_Rng.GetRandomF32SignedInRange(0.033333335f);
        if (speed < -0.06666667f)
        {
            speed = -0.06666667f;
        }
        else if (speed > 0.06666667f)
        {
            speed = 0.06666667f;
        }
        SetPolar((Vector2 *)&data->disc.points[i], angle, data->disc.radii[i]);
        data->disc.points[i] += data->disc.base.targetPos;
        angle += 0.2026834f;
    }
    return 0;
}

// The draw callback of extra attack types 19 and 21: once the attack
// has landed, the disc as a textured fan through its points, the centre
// at its sprite's alpha and the rim in light red, closed on the first
// rim point. Clears unknown684. Returns 0 (0x44b940).
int ExAttackDraw44b940(ExAttack *attack)
{
    ExAttackData19 *data = (ExAttackData19 *)attack->data;

    if (data->base.state >= 1)
    {
        BYTE alpha = attack->sprites->alpha;
        VertexTex1DiffuseXyzrwh *vtx = data->vertices;
        float angle = -3.1415927f;
        int i;

        vtx->pos = data->points[0];
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->diffuse = (alpha << 24) | 0xffffff;
        vtx++;
        for (i = 1; i < 0x20; i++)
        {
            if (angle >= 3.1415927f)
            {
                angle -= 6.2831855f;
            }
            vtx->pos.z = 0.0f;
            vtx->pos = data->points[i];
            vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
            vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
            vtx->diffuse = 0x8080ff;
            vtx++;
            angle += 0.2026834f;
        }
        *vtx = data->vertices[1];
        g_SpriteEngine->DrawVertexFan(attack->sprites, data->vertices, 0x21);
    }
    data->unknown684 = 0;
    return 0;
}
