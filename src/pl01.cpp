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

// The unknown30408 of all characters but 2 and 4: gives the enemy a
// random sideways velocity2 and one upwards that grows over 120 frames
// of its unknown5424. Returns 0 (0x4418b0).
int PlayerEnemyFunc4418b0(Player *player, Enemy *enemy)
{
    enemy->velocity2.x = g_Rng.GetRandomF32SignedInRange(0.1f);
    enemy->velocity2.y = enemy->unknown5424 > 0x78 ? -1.0f : enemy->unknown5424.AsFloat() * (-1.0f / 120.0f);
    enemy->velocity2.z = 0.0f;
    enemy->unknown5424++;
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

// The update callback of extra attack type 2: flies in along the curve
// for 90 frames, then spawns enemy subroutine 8 of the target's player
// ECL there, its first float variable -pi/2. Returns 1 once it did
// (0x441a70).
int ExAttackUpdate441a70(ExAttack *attack)
{
    ExAttackData00 *data = (ExAttackData00 *)attack->data;
    EclVars vars;

    if (data->state == 0)
    {
        HermiteInterpolate(&attack->pos, &data->end, &data->start, &data->tangent0, &data->tangent1,
                           attack->timer.AsFloat(), 90.0f);
        if (attack->timer > 0x5a)
        {
            vars.floatVars[0] = -1.5707964f;
            g_GameManager.sides[attack->targetSide].enemyManager->SpawnPlayerEnemyWithVars(8, &data->targetPos, 0x14,
                                                                                            -2, 1000, &vars);
            return 1;
        }
    }
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
