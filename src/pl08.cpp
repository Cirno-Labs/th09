// Character 8's extra attacks and callbacks (pl/pl08.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30450 of character 8: while unknown34 reaches 100 -
// unknownB4 * 3, sends an extra attack of type 16 from `pos` and takes
// as much off it. Returns 0 (0x447a80).
int PlayerComboFunc447a80(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= 100 - g_GameManager.unknownB4 * 3)
    {
        g_GameManager.exAttackManager->AddAttack(16, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= 100 - g_GameManager.unknownB4 * 3;
    }
    return 0;
}

// An init callback of the effect table of types 10, 18, 24, 25, 26, 28 and 33: allocates 0x294 bytes of
// vertices and sets the effect up as a ring of 32 segments, radius 16,
// around its position, from the angle in velocity.x, as wide as
// velocity.z. Returns 0, or -1 if the allocation fails (0x447c40).
int EffectInit447c40(Effect *effect)
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
    effect->unknown7c = 16.0f;
    effect->unknownC9 = 1;
    return 0;
}
