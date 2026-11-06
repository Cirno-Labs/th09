// Character 2's extra attacks and callbacks (pl/pl02.cpp).

#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "pl.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

// The init callback of effect type 16: InitEffectTypes10And18And24And25And26And28And33's ring of
// radius 16, with unknown84 at 0.01. Returns 0, or -1 if the allocation
// fails (0x441c80).
int InitEffectType16(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryTracker.Alloc(0x294, "Effect");
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
    effect->unknown7c = 16.0f;
    effect->unknownC9 = 1;
    effect->unknown84 = 0.01f;
    return 0;
}

// An update callback of the effect table: after 30 frames, grows
// unknown84 by 0.001 up to 0.08; sets unknownC9 and puts the effect on
// its side's player. Returns 1 (0x441d30).
int UpdateEffectType16(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->timer >= 0x1e)
    {
        if (effect->unknown84 < 0.08f)
        {
            effect->unknown84 += 0.001f;
        }
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}

// The draw callback of effect type 16: a fan from 432 above the effect
// down over a half circle 448 in radius, squeezed across by unknown84,
// if unknownC9 is set. Returns 1 (0x441da0).
int DrawEffectType16(Effect *effect)
{
    if (effect->unknownC9)
    {
        float delta = 3.1415927f / (effect->unknown8c - 1);
        VertexDiffuseXyzrwh *vtx = (VertexDiffuseXyzrwh *)effect->vertices;
        float angle = 0.0f;
        int i;

        vtx->pos = effect->pos;
        vtx->pos.y -= 432.0f;
        vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
        vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
        vtx->w = 1.0f;
        vtx->diffuse = 0x30600040;
        vtx++;
        for (i = effect->unknown8c; i > 0; i--)
        {
            vtx->pos.z = 0.0f;
            SetPolar((Vector2 *)&vtx->pos, angle, 448.0f);
            vtx->pos.x *= effect->unknown84;
            vtx->pos += effect->pos;
            vtx->pos.y -= 432.0f;
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

// A fire callback of the shot table (1): on the frame of `sd`, sets the
// player's unknown3031c to the bullet for 30 frames, keeps the shot's
// option and offset in the bullet, then fires it as type 2 with an empty
// trail.
// Returns 1 if it fired (0x441ef0).
int FirePlayerShotType1(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    int i;

    if (frame != sd->frame)
    {
        return 0;
    }
    player->unknown3031c.timer = 30;
    player->unknown3031c.bullet = bullet;
    bullet->unknown468 = sd->option;
    bullet->velocity.z = sd->offsetX;
    bullet->unknown448 = sd->offsetY;
    bullet->unknown46a = 0xf;
    FireShot(player, bullet, sd);
    bullet->type = 2;
    for (i = 0x1f; i >= 0; i--)
    {
        bullet->trail[i].x = -999.0f;
    }
    bullet->pos.x = -999.0f;
    return 1;
}

// An update callback of the shot table (11): keeps the bullet on the
// player, offset by velocity.z and half as high, with its trail falling
// behind it, and adds a 4 by 448 box at every other trail point, as many
// as unknown46a, while the player's unknown3031c timer runs; then frees
// the bullet. Returns 1 once freed, otherwise 0 (0x441f90).
int UpdatePlayerShotType2(Player *player, PlayerBullet *bullet)
{
    int i;

    if (player->unknown3031c.timer <= 0)
    {
        player->unknown3031c.timer = 0;
        player->unknown3031c.bullet = NULL;
        bullet->state = 0;
        return 1;
    }
    bullet->pos = player->pos;
    bullet->pos.x += bullet->velocity.z;
    bullet->pos.z = 0.44f;
    bullet->size.y = bullet->pos.y;
    bullet->sprite.scaleY = bullet->pos.y * 0.071428575f;
    bullet->pos.y *= 0.5f;
    for (i = 0x1f; i > 0; i--)
    {
        bullet->trail[i] = bullet->trail[i - 1];
        bullet->trail[i].y -= 1.0f;
    }
    for (i = 0; i < bullet->unknown46a; i++)
    {
        if (bullet->trail[i * 2].x >= -900.0f)
        {
            player->AddKind4Box(&bullet->trail[i * 2], 4.0f, 448.0f, 1, 1, 0);
        }
    }
    bullet->trail[0] = bullet->pos;
    player->unknown3031c.timer--;
    return 0;
}

// A draw callback of the shot table (18): draws the bullet's sprite at
// each point of its trail up to the first unused one, twice
// unknown46a of them, fading out along it. Returns 0 (0x442220).
int DrawPlayerShotType1(Player *player, PlayerBullet *bullet)
{
    int alpha = bullet->sprite.alpha;
    int i;

    for (i = 0; i < bullet->unknown46a * 2; i++)
    {
        // -999.0f, compared by its bits.
        if (*(DWORD *)&bullet->trail[i].x == 0xc479c000)
        {
            break;
        }
        bullet->sprite.pos.x = g_GameManager.ArenaToScreenX(bullet->trail[i].x);
        bullet->sprite.pos.y = g_GameManager.ArenaToScreenY(bullet->trail[i].y);
        bullet->sprite.pos.z = 0.0f;
        bullet->sprite.alpha = alpha - alpha * i / (bullet->unknown46a * 2);
        g_SpriteEngine->Draw(&bullet->sprite);
    }
    bullet->sprite.alpha = alpha;
    return 0;
}

// The update callback of effect type 17: grows unknown84 by pi/60 up to
// pi/2, sets unknownC9, puts the effect on its side's player and turns
// unknown80 a quarter of the shorter way towards opposite the player's
// unknown1cd8. Returns 1 (0x4422f0).
int UpdateEffectType17(Effect *effect)
{
    float target;
    float delta;

    effect->unknownC9 = 1;
    if (effect->unknown84 < 1.5707964f)
    {
        effect->unknown84 += 0.05235988f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    target = AddNormalizeAngle(effect->mgr->side->player->unknown1cd8, 3.1415927f);
    delta = target - effect->unknown80;
    if (target > effect->unknown80)
    {
        if (delta > 3.1415927f)
        {
            delta -= 6.2831855f;
        }
    }
    else
    {
        if (delta < -3.1415927f)
        {
            delta += 6.2831855f;
        }
    }
    effect->unknown80 = AddNormalizeAngle(effect->unknown80, delta * 0.25f);
    return 1;
}

// A fire callback of the shot table (2): fires `sd` on its frame,
// standing still at its angle. Returns 1 if it fired (0x4423d0).
int FirePlayerShotType2(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame == sd->frame)
    {
        FireShot(player, bullet, sd);
        bullet->velocity.x = 0.0f;
        bullet->velocity.y = 0.0f;
        bullet->angle = sd->angle;
        return 1;
    }
    return 0;
}

// The unknown30404 of characters 2 and 10: whether `pos` is within the
// radius of the player's unknown368 and the arc of unknown84 radians
// about its angle unknown80 (0x442480).
int CheckFocusAreaCharacters2And10(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    Effect *circle = player->unknown368;
    float angle;

    if (diff.x * diff.x + diff.y * diff.y < circle->unknown7c * circle->unknown7c)
    {
        angle = player->AngleFromPlayer(pos) - circle->unknown80;
        if (Fabs(angle) > 3.1415927f)
        {
            angle = angle > 0.0f ? angle - 6.2831855f : angle + 6.2831855f;
        }
        if (Fabs(angle) <= circle->unknown84 * 0.5f)
        {
            return 1;
        }
    }
    return 0;
}

// The unknown30408 of character 2: stops the enemy's velocity2.
// Returns 0 (0x442550).
int UpdateFocusedEnemyCharacter2(Player *player, Enemy *enemy)
{
    enemy->velocity2.x = 0.0f;
    enemy->velocity2.y = 0.0f;
    enemy->velocity2.z = 0.0f;
    enemy->unknown5424++;
    return 0;
}

// The data of extra attack types 3 and 4 (0x190 bytes).
struct ExAttackData03
{
    int state;
    D3DXVECTOR3 unknown04;
    // Where it was, newest first.
    D3DXVECTOR3 trail[0x20];
};
ASSERT_SIZE(ExAttackData03, 0x190);

// The init callback of extra attack type 3, character 2's: five
// sprites, from scripts 9 to 13, flying on screen from where it is
// towards the target's player at 3.5, with an attack of type 4 sent
// along for the target's side. Returns 0 (0x442580).
int InitExAttackType3(ExAttack *attack)
{
    ExAttackData03 *data;
    D3DXVECTOR3 targetPos;
    D3DXVECTOR3 pos;
    float angle;
    int i;

    attack->AllocSprites(5, sizeof(ExAttackData03));
    data = (ExAttackData03 *)attack->data;
    attack->GetSide()->player->anm->StartScriptAtOrigin(&attack->sprites[0], 9);
    attack->GetSide()->player->anm->StartScriptAtOrigin(&attack->sprites[1], 10);
    attack->GetSide()->player->anm->StartScriptAtOrigin(&attack->sprites[2], 11);
    attack->GetSide()->player->anm->StartScriptAtOrigin(&attack->sprites[3], 12);
    attack->GetSide()->player->anm->StartScriptAtOrigin(&attack->sprites[4], 13);
    g_App.SetCamera(attack->targetSide);
    targetPos.x = g_GameManager.ArenaToScreenX(g_GameManager.sides[attack->targetSide].player->pos.x);
    targetPos.y = g_GameManager.ArenaToScreenY(g_GameManager.sides[attack->targetSide].player->pos.y);
    g_App.SetCamera(attack->side);
    pos.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    pos.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    pos.z = 0.0f;
    attack->pos = pos;
    attack->layer = 2;
    angle = Atan2(targetPos.y - pos.y, targetPos.x - pos.x);
    attack->angle = angle;
    SetPolar((Vector2 *)&data->unknown04, angle, 3.5f);
    data->state = 0;
    g_GameManager.exAttackManager->AddAttack(4, &pos, attack->side, (D3DXVECTOR3 *)attack);
    for (i = 0; i < 0x20; i++)
    {
        data->trail[i] = attack->pos;
    }
    return 0;
}

// The update callback of extra attack type 3: drags its trail behind it,
// the other four sprites on every sixth point of it, flying on by
// unknown04 after 30 frames until the trail's end leaves the screen; if
// it, or the trail's points 12 or 24, touches the target's player, its
// sprites are interrupted and it ends 15 frames later. Returns 1 once it
// is done (0x442750).
int UpdateExAttackType3(ExAttack *attack)
{
    ExAttackData03 *data = (ExAttackData03 *)attack->data;
    D3DXVECTOR3 pos0;
    D3DXVECTOR3 pos1;
    D3DXVECTOR3 pos2;
    int i;

    switch (data->state)
    {
    case 0:
    {
        for (i = 0x1f; i > 0; i--)
        {
            data->trail[i] = data->trail[i - 1];
        }
        data->trail[0] = attack->pos;
        attack->sprites[1].posOffset = data->trail[6] - attack->pos;
        attack->sprites[2].posOffset = data->trail[12] - attack->pos;
        attack->sprites[3].posOffset = data->trail[18] - attack->pos;
        attack->sprites[4].posOffset = data->trail[24] - attack->pos;
        D3DXVECTOR3 size;
        float *s = size;
        s[0] = 12.0f;
        s[1] = 12.0f;
        s[2] = 0.0f;
        g_App.SetCamera(attack->targetSide);
        pos0.x = g_GameManager.ScreenToArenaX(attack->pos.x);
        pos0.y = g_GameManager.ScreenToArenaY(attack->pos.y);
        pos0.z = 0.0f;
        pos1.x = g_GameManager.ScreenToArenaX(data->trail[12].x);
        pos1.y = g_GameManager.ScreenToArenaY(data->trail[12].y);
        pos1.z = 0.0f;
        pos2.x = g_GameManager.ScreenToArenaX(data->trail[24].x);
        pos2.y = g_GameManager.ScreenToArenaY(data->trail[24].y);
        pos2.z = 0.0f;
        if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&pos0, &size, NULL) == 2 ||
            g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&pos1, &size, NULL) == 2 ||
            g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&pos2, &size, NULL) == 2)
        {
            attack->timer = 0;
            data->state = 1;
            attack->sprites[0].SetPendingInterrupt(1);
            attack->sprites[1].SetPendingInterrupt(1);
            attack->sprites[2].SetPendingInterrupt(1);
            attack->sprites[3].SetPendingInterrupt(1);
            attack->sprites[4].SetPendingInterrupt(1);
            break;
        }
        if (attack->timer >= 30)
        {
            attack->pos += data->unknown04;
            if (data->trail[0x1f].x < -32.0f || data->trail[0x1f].x > 672.0f || data->trail[0x1f].y < -32.0f ||
                data->trail[0x1f].y > 512.0f)
            {
                return 1;
            }
        }
        break;
    }
    case 1:
        if (attack->timer >= 15)
        {
            return 1;
        }
        break;
    }
    return 0;
}

// The init callback of extra attack type 4: five sprites, from scripts
// 14 to 18, on the target's side at the screen position it was sent
// from, starting from the attack it was sent for: its angle and
// unknown04. Returns 0 (0x442a60).
int InitExAttackType4(ExAttack *attack)
{
    ExAttackData03 *data;
    ExAttack *parent;
    int i;

    attack->spriteCount = 5;
    attack->sprites = (Sprite *)g_MemoryTracker.Alloc(sizeof(Sprite) * 5, "./system\\global.h");
    g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[0], 0xe);
    g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[1], 0xf);
    g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[2], 0x10);
    g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[3], 0x11);
    g_GameManager.sides[attack->side].player->anm->StartScriptAtOrigin(&attack->sprites[4], 0x12);
    data = (ExAttackData03 *)g_MemoryTracker.Alloc(sizeof(ExAttackData03), "./system\\global.h");
    attack->data = data;
    g_App.SetCamera(attack->targetSide);
    attack->pos.x = g_GameManager.ScreenToArenaX(attack->pos.x);
    attack->pos.y = g_GameManager.ScreenToArenaY(attack->pos.y);
    attack->layer = attack->targetSide;
    parent = (ExAttack *)attack->target;
    attack->angle = parent->angle;
    data->unknown04 = ((ExAttackData03 *)parent->data)->unknown04;
    data->state = 0;
    for (i = 0; i < 0x20; i++)
    {
        data->trail[i] = attack->pos;
    }
    return 0;
}

// The update callback of extra attack type 4: type 3's twin on the
// target's side, dragging its trail and sprites the same way and, after
// 30 frames, flying on by unknown04 as five hit circles on its trail
// until the parent's trail point 24 leaves the screen. Once the parent
// is gone or has hit, its sprites are interrupted and it ends 15 frames
// later. Returns 1 once it is done (0x442bd0).
int UpdateExAttackType4(ExAttack *attack)
{
    ExAttackData03 *data = (ExAttackData03 *)attack->data;
    int i;

    switch (data->state)
    {
    case 0:
    {
        for (i = 0x1f; i > 0; i--)
        {
            data->trail[i] = data->trail[i - 1];
        }
        data->trail[0] = attack->pos;
        attack->sprites[1].posOffset = data->trail[6] - attack->pos;
        attack->sprites[2].posOffset = data->trail[12] - attack->pos;
        attack->sprites[3].posOffset = data->trail[18] - attack->pos;
        attack->sprites[4].posOffset = data->trail[24] - attack->pos;
        ExAttackData03 *parentData = (ExAttackData03 *)((ExAttack *)attack->target)->data;
        D3DXVECTOR3 size;
        float *s = size;
        s[0] = 12.0f;
        s[1] = 12.0f;
        s[2] = 0.0f;
        if (parentData == NULL || parentData->state != 0)
        {
            attack->timer = 0;
            data->state = 1;
            attack->sprites[0].SetPendingInterrupt(1);
            attack->sprites[1].SetPendingInterrupt(1);
            attack->sprites[2].SetPendingInterrupt(1);
            attack->sprites[3].SetPendingInterrupt(1);
            attack->sprites[4].SetPendingInterrupt(1);
            break;
        }
        if (attack->timer >= 30)
        {
            attack->pos += data->unknown04;
            g_GameManager.sides[attack->targetSide].player->unknown36c.AddCircle(&attack->pos, 12.0f, NULL);
            g_GameManager.sides[attack->targetSide].player->unknown36c.AddCircle(&data->trail[6], 12.0f, NULL);
            g_GameManager.sides[attack->targetSide].player->unknown36c.AddCircle(&data->trail[12], 12.0f, NULL);
            g_GameManager.sides[attack->targetSide].player->unknown36c.AddCircle(&data->trail[18], 12.0f, NULL);
            g_GameManager.sides[attack->targetSide].player->unknown36c.AddCircle(&data->trail[24], 12.0f, NULL);
            if (parentData->trail[24].x < -32.0f || parentData->trail[24].x > 672.0f ||
                parentData->trail[24].y < -32.0f || parentData->trail[24].y > 512.0f)
            {
                return 1;
            }
        }
        break;
    }
    case 1:
        if (attack->timer >= 15)
        {
            return 1;
        }
        break;
    }
    return 0;
}

// The free callback of extra attack type 4: frees the data block, which
// its init allocates apart from the sprites. Returns 0 (0x442ec0).
int FreeExAttackType4(ExAttack *attack)
{
    if (attack->data != NULL)
    {
        g_MemoryTracker.Free(attack->data);
    }
    attack->data = NULL;
    return 0;
}

// The combo func of character 2: while unknown34 reaches
// (45 - unknownB4) * 2, sends an extra attack of type 3 from `pos` and
// takes as much off it. Returns 0 (0x442ee0).
int SendComboAttacksCharacter2(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (45 - g_GameManager.rank.unknownB4) * 2)
    {
        g_GameManager.exAttackManager->AddAttack(3, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (45 - g_GameManager.rank.unknownB4) * 2;
    }
    return 0;
}

// The init callback of effect type 17: a ring of 16 segments, radius
// 160, around its position, from opposite the player's unknown1cd8, as
// wide as velocity.z, in 0x294 bytes of vertices. Returns 0, or -1 if
// the allocation fails (0x442f50).
int InitEffectType17(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryTracker.Alloc(0x294, "Effect");
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
    effect->unknown7c = 160.0f;
    effect->unknownC9 = 1;
    return 0;
}

// The draw callback of effect type 17: a fan of unknown8c - 1 triangles
// about the effect, unknown7c in radius, over unknown84 radians centred
// on unknown80, if unknownC9 is set. Returns 1 (0x443010).
int DrawEffectType17(Effect *effect)
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

// An update callback of the shot table (12): 30 frames after it fires,
// aims a flying bullet at the player's orbHomingTarget, turned by its
// angle, or straight up if that is left of the play area.
// Returns 0 (0x443150).
int UpdatePlayerShotType3(Player *player, PlayerBullet *bullet)
{
    float angle;
    float dx;
    float dy;

    if (bullet->state == 1 && bullet->timer.AsFrames() == 30)
    {
        if (player->orbHomingTarget.x >= -144.0f)
        {
            dx = player->orbHomingTarget.x - bullet->pos.x;
            dy = player->orbHomingTarget.y - bullet->pos.y;
            if (dy == 0.0f && dx == 0.0f)
            {
                angle = bullet->angle + 1.5707964f;
            }
            else
            {
                angle = Atan2(dy, dx) + bullet->angle;
            }
        }
        else
        {
            angle = -1.5707964f;
        }
        angle = AddNormalizeAngle(angle, 0.0f);
        bullet->velocity.x = cosf(angle) * bullet->speed;
        bullet->velocity.y = sinf(angle) * bullet->speed;
        bullet->angle = angle;
    }
    return 0;
}
