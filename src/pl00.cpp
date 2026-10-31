// Character 0's spell cards, extra attacks and callbacks (pl/pl00.cpp).

#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "spellcard.h"

// The update callback of spell card types 2, 5 and 8: whether no enemy of
// unknownType 3 is left on the side (0x440cc0).
int SpellCardUpdate440cc0(SpellCard *card)
{
    return g_GameManager.sides[card->sideIdx].enemyManager->FindByUnknownType(3) == NULL;
}

// An update callback of the effect table: grows unknown7c by 4 up to 96,
// sets unknownC9 and puts the effect on its side's player. Returns 1
// (0x440ce0).
int EffectUpdate440ce0(Effect *effect)
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
int PlayerPosFunc440df0(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;
    float radius = player->unknown368->unknown7c;

    if (diff.x * diff.x + diff.y * diff.y < radius * radius)
    {
        return 1;
    }
    return 0;
}

// The data of extra attack types 0 and 1 (0x4c bytes).
struct ExAttackData00
{
    int state;
    // Its speed while it falls; x and y start along a random angle.
    D3DXVECTOR3 velocity;
    // The Hermite curve it flies in on: from start to end on screen, with
    // two random tangents.
    D3DXVECTOR3 start;
    D3DXVECTOR3 end;
    D3DXVECTOR3 tangent1;
    D3DXVECTOR3 tangent0;
    // Where it appears on the target's side.
    D3DXVECTOR3 targetPos;
};

// The init callback of extra attack type 0: one sprite, flying in to a
// random spot on the upper half of the other side, a curve whose end it
// places in that side's view. Returns 0 (0x440e40).
int ExAttackInit440e40(ExAttack *attack)
{
    ExAttackData00 *data;
    float angle;

    attack->AllocSprites(1, sizeof(ExAttackData00));
    attack->GetSide()->effectManager->anm->StartScriptAtOrigin(attack->sprites, 0x25);
    data = (ExAttackData00 *)attack->data;
    angle = g_Rng.GetRandomF32InRange(-3.1415927f);
    data->velocity.z = 0.0f;
    ((Vector2 *)&data->velocity)->SetPolar(angle, 2.0f);
    data->state = 0;
    attack->layer = 2;
    ((Vector2 *)&data->tangent1)->SetPolar(g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
    ((Vector2 *)&data->tangent0)->SetPolar(g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(192.0f));
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

// The init callbacks of spell card types 0 and 1: allocate unknown1360
// and spawn enemy subroutine 0 or 1 from the player's ECL (0x441350,
// 0x4413d0). The position is never set. Return 0.
int SpellCardInit441350(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unknown1360 = g_MemoryManager.Alloc(0x2b8, ".\\pl\\pl00.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(0, &pos, 0x1e, -2, 1000, 0);
    return 0;
}

// The free callback of spell card types 0, 1, 3, 4, 6 and 7: frees
// unknown1360. Returns 0 (0x4413a0).
int SpellCardFree4413a0(SpellCard *card)
{
    if (card->unknown1360 != NULL)
    {
        g_MemoryManager.Free(card->unknown1360);
    }
    card->unknown1360 = NULL;
    return 0;
}

int SpellCardInit4413d0(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unknown1360 = g_MemoryManager.Alloc(0x2b8, ".\\pl\\pl00.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(1, &pos, 0x1e, -2, 10000, 0);
    return 0;
}

// The init callback of spell card type 2: allocates unknown1364 and
// spawns enemy subroutine 2 from the player's ECL, with the position
// never set. Returns 0 (0x441420).
int SpellCardInit441420(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unknown1364 = g_MemoryManager.Alloc(0x2b8, ".\\pl\\pl00.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(2, &pos, 0x578, -2, 1000, 0);
    return 0;
}

// The free callback of spell card types 2, 5 and 8: frees unknown1364.
// Returns 0 (0x441470).
int SpellCardFree441470(SpellCard *card)
{
    if (card->unknown1364 != NULL)
    {
        g_MemoryManager.Free(card->unknown1364);
    }
    card->unknown1364 = NULL;
    return 0;
}
