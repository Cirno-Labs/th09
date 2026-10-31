// Character 1's spell cards, extra attacks and callbacks (pl/pl01.cpp).

#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"
#include "spellcard.h"
#include "unsorted.h"

// The unknown30404 of character 1: whether `pos` is above the player,
// within a parabola about it that the player's unknown368 widens by its
// unknown84, and at least 14 across (0x4417f0).
int PlayerPosFunc4417f0(Player *player, D3DXVECTOR3 *pos)
{
    float top = player->pos.y + 32.0f;
    float dx;
    float width;

    if (top <= pos->y)
    {
        return 0;
    }
    top = pos->y - (top - 448.0f);
    dx = pos->x - player->pos.x;
    width = (200704.0f - top * top) * player->unknown368->unknown84 * player->unknown368->unknown84;
    if (width < 196.0f)
    {
        width = 196.0f;
    }
    if (dx * dx <= width)
    {
        return 1;
    }
    return 0;
}

// A hit callback of the shot table (22): counts unknown30354 up.
// Returns 0 (0x441880).
int ShotHit441880(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos)
{
    player->unknown30354++;
    return 0;
}

// The init callback of extra attack type 2: pl00's type 0, flying in to
// a random spot on the bottom of the other side. Returns 0 (0x441910).
int ExAttackInit441910(ExAttack *attack)
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
    data->targetPos.y = 448.0f;
    data->targetPos.z = 0.0f;
    g_App.SetCamera(attack->targetSide);
    data->end.x = g_GameManager.ArenaToScreenX(data->targetPos.x);
    data->end.y = g_GameManager.ArenaToScreenY(data->targetPos.y);
    data->end.z = 0.0f;
    g_App.SetCamera(attack->side);
    return 0;
}

// The init callbacks of spell card types 3 and 4, pl00's for types 0 and
// 1 (0x441b70, 0x441be0).
int SpellCardInit441b70(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unknown1360 = g_MemoryManager.Alloc(0x2b8, ".\\pl\\pl01.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(0, &pos, 0x1e, -2, 1000, 0);
    return 0;
}

// The update callback of spell card types 0, 1, 3, 4, 6 and 7: whether
// unknown98 passed 192 frames (0x441bc0).
int SpellCardUpdate441bc0(SpellCard *card)
{
    if (card->unknown98 > 0xc0)
    {
        return 1;
    }
    return 0;
}

int SpellCardInit441be0(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unknown1360 = g_MemoryManager.Alloc(0x2b8, ".\\pl\\pl01.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(1, &pos, 0x1e, -2, 10000, 0);
    return 0;
}

// The init callback of spell card type 5, pl00's for type 2 (0x441c30).
int SpellCardInit441c30(SpellCard *card)
{
    D3DXVECTOR3 pos;

    card->unknown1364 = g_MemoryManager.Alloc(0x2b8, ".\\pl\\pl01.cpp");
    g_GameManager.sides[card->sideIdx].enemyManager->SpawnPlayerEnemy(2, &pos, 0x578, -2, 1000, 0);
    return 0;
}
