// Character 15's extra attacks and callbacks (pl/pl15.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30404 of character 15: whether `pos` is within the radius
// of the player's unknown368 across or up or down (0x44c2e0).
int PlayerPosFunc44c2e0(Player *player, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 diff = player->pos - *pos;

    if (diff.x * diff.x < player->unknown368->unknown7c * player->unknown368->unknown7c ||
        diff.y * diff.y < player->unknown368->unknown7c * player->unknown368->unknown7c)
    {
        return 1;
    }
    return 0;
}

// The unknown30450 of character 15: while unknown34 reaches 80 -
// unknownB4 * 3, sends an extra attack of type 26 from `pos` and takes
// as much off it. Returns 0 (0x44c680).
int PlayerComboFunc44c680(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 80 - g_GameManager.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(26, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 80 - g_GameManager.unknownB4 * 3;
    }
    return 0;
}

// The init callback of effect type 37: EffectInit447c40's ring with
// 0xf0 bytes of vertices and a radius of 0. Returns 0, or -1 if the
// allocation fails (0x44c6f0).
int EffectInit44c6f0(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryManager.Alloc(0xf0, "Effect");
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
    effect->unknown7c = 0.0f;
    effect->unknownC9 = 1;
    return 0;
}
