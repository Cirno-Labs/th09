// Character 0's spell cards, extra attacks and callbacks (pl/pl00.cpp).

#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "pl.h"
#include "player.h"
#include "spellcard.h"
#include "sprite.h"
#include "unsorted.h"

// The update callback of spell card types 2, 5 and 8: whether no enemy of
// unknownType 3 is left on the side (0x440cc0).
int IsSpellCardEnemyDefeated(SpellCard *card)
{
    return g_GameManager.sides[card->sideIdx].enemyManager->FindByUnknownType(3) == NULL;
}

// An update callback of the effect table: grows unknown7c by 4 up to 96,
// sets unknownC9 and puts the effect on its side's player. Returns 1
// (0x440ce0).
int UpdateEffectType10(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknown7c < 96.0f)
    {
        effect->unknown7c += 4.0f;
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}

// The unknown30404 of characters 0, 3, 5, 12 and 13: whether `pos` is
// within the radius of the player's unknown368 (0x440df0).
int IsWithinCircularFocusArea(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    float radius = player->unknown368->unknown7c;

    if (diff.x * diff.x + diff.y * diff.y < radius * radius)
    {
        return 1;
    }
    return 0;
}

// The init callback of extra attack type 0: one sprite, flying in to a
// random spot on the upper half of the other side, a curve whose end it
// places in that side's view. Returns 0 (0x440e40).
int InitExAttackType0(ExAttack *attack)
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
    data->targetPos.y = g_Rng.GetRandomF32InRange(128.0f);
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}

// The init callback of extra attack type 1: type 0's, flying in to near
// its own position on screen after waiting 90 frames. Returns 0
// (0x440fb0).
int InitExAttackType1(ExAttack *attack)
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
    attack->timer = 0x5a;
    attack->layer = 2;
    SetPolar((Vector2 *)&data->tangent1,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    SetPolar((Vector2 *)&data->tangent0,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    data->start.x = g_GameManager.ArenaToScreenX(attack->pos.x);
    data->start.y = g_GameManager.ArenaToScreenY(attack->pos.y);
    data->start.z = 0.0f;
    data->targetPos.x = g_Rng.GetRandomF32SignedInRange(64.0f) + attack->pos.x;
    data->targetPos.y = g_Rng.GetRandomF32SignedInRange(64.0f) + attack->pos.x;
    data->targetPos.z = 0.0f;
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    return 0;
}

// The update callback of extra attack types 0 and 1: flies in along the
// curve for 90 frames, then appears on the target's side and, after 20
// frames, falls bouncing off the sides as a hit circle until it touches
// the player there or leaves the bottom. Returns 1 once it is done
// (0x441100).
int UpdateExAttackTypes0And1(ExAttack *attack)
{
    ExAttackData00 *data = (ExAttackData00 *)attack->data;
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
            if (g_GameManager.sides[attack->targetSide].player->CheckHitAreas(&attack->pos, &size, NULL) == 2)
            {
                data->state = 2;
                attack->timer = 0;
                attack->sprites->SetPendingInterrupt(1);
                return 0;
            }
            attack->pos += data->velocity;
            if ((data->velocity.x < 0.0f && attack->pos.x < -144.0f) ||
                (data->velocity.x > 0.0f && attack->pos.x > 144.0f))
            {
                data->velocity.x = -data->velocity.x;
            }
            data->velocity.y += 0.04f;
            if (data->velocity.y > 3.0f)
            {
                data->velocity.y = 3.0f;
            }
            g_GameManager.sides[attack->targetSide].player->unknown36c.AddCircle(&attack->pos, 18.0f, NULL);
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

// The combo func of character 0: while unknown34 reaches
// (28 - unknownB4) * 5, sends an extra attack of type 0 from `pos` and
// takes as much off it. Returns 0 (0x4412e0).
int SendComboAttacksCharacter0(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (28 - g_GameManager.rank.unknownB4) * 5)
    {
        g_GameManager.exAttackManager->AddAttack(0, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (28 - g_GameManager.rank.unknownB4) * 5;
    }
    return 0;
}

// The init callbacks of spell card types 0 and 1: allocate unknown1360
// and spawn enemy subroutine 0 or 1 from the player's ECL (0x441350,
// 0x4413d0). The position is never set. Return 0.
int InitSpellCardType0(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unknown1360 = g_MemoryTracker.Alloc(0x2b8, ".\\pl\\pl00.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(0, &pos, 0x1e, -2, 1000, 0);
    return 0;
}

// The free callback of spell card types 0, 1, 3, 4, 6 and 7: frees
// unknown1360. Returns 0 (0x4413a0).
int FreePrimarySpellCardData(SpellCard *card)
{
    if (card->unknown1360 != NULL)
    {
        g_MemoryTracker.Free(card->unknown1360);
    }
    card->unknown1360 = NULL;
    return 0;
}

int InitSpellCardType1(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unknown1360 = g_MemoryTracker.Alloc(0x2b8, ".\\pl\\pl00.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(1, &pos, 0x1e, -2, 10000, 0);
    return 0;
}

// The init callback of spell card type 2: allocates unknown1364 and
// spawns enemy subroutine 2 from the player's ECL, with the position
// never set. Returns 0 (0x441420).
int InitSpellCardType2(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unknown1364 = g_MemoryTracker.Alloc(0x2b8, ".\\pl\\pl00.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(2, &pos, 0x578, -2, 1000, 0);
    return 0;
}

// The free callback of spell card types 2, 5 and 8: frees unknown1364.
// Returns 0 (0x441470).
int FreeSecondarySpellCardData(SpellCard *card)
{
    if (card->unknown1364 != NULL)
    {
        g_MemoryTracker.Free(card->unknown1364);
    }
    card->unknown1364 = NULL;
    return 0;
}

// The draw callback of effect type 10: a fan of unknown8c - 1 triangles
// about the effect, unknown7c in radius, if unknownC9 is set. Returns 1
// (0x4414a0).
int DrawEffectType10(Effect *effect)
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

// An update callback of the shot table (1): from 40 frames on, every
// other frame, steers a flying bullet towards the player's
// orbHomingTarget, by up to a quarter of its speed and at 1 to 10 pixels
// a frame, or speeds it up by a third up to 10 without a target; turns
// it to its velocity. Returns 0 (0x4415e0).
int UpdatePlayerShotType1(Player *player, PlayerBullet *bullet)
{
    float x;
    float y;
    float len;

    if (bullet->state == 1 && bullet->timer.AsFrames() >= 40 && bullet->timer.HasTicked())
    {
        if (player->orbHomingTarget.x > -900.0f)
        {
            x = player->orbHomingTarget.x - bullet->pos[0];
            y = player->orbHomingTarget.y - bullet->pos[1];
            len = sqrtf(x * x + y * y) / (bullet->speed * 0.25f);
            if (len < 1.0f)
            {
                len = 1.0f;
            }
            x = x / len + bullet->velocity.x;
            y = y / len + bullet->velocity.y;
            len = sqrtf(x * x + y * y);
            bullet->speed = len > 10.0f ? 10.0f : len;
            if (bullet->speed < 1.0f)
            {
                bullet->speed = 1.0f;
            }
            bullet->velocity.x = x * bullet->speed / len;
            bullet->velocity.y = y * bullet->speed / len;
        }
        else if (bullet->speed < 10.0f)
        {
            bullet->speed += 0.33333334f;
            x = bullet->velocity.x;
            y = bullet->velocity.y;
            len = sqrtf(x * x + y * y);
            bullet->velocity.x = x * bullet->speed / len;
            bullet->velocity.y = y * bullet->speed / len;
        }
    }
    bullet->angle = Atan2(bullet->velocity.y, bullet->velocity.x);
    return 0;
}
