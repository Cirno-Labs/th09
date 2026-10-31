// Character 11's extra attacks and callbacks (pl/pl11.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30450 of character 11: while unknown34 reaches (28 -
// unknownB4) * 5, sends an extra attack of type 19 from `pos` and
// takes as much off it. Returns 0 (0x44a670).
int PlayerComboFunc44a670(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (28 - g_GameManager.unknownB4) * 5)
    {
        g_GameManager.exAttackManager->AddAttack(19, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (28 - g_GameManager.unknownB4) * 5;
    }
    return 0;
}

// The init callback of effect type 31: EffectInit447c40's ring of
// radius 16, from -pi/2. Returns 0, or -1 if the allocation fails (0x44a6e0).
int EffectInit44a6e0(Effect *effect)
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
    effect->unknown88 = effect->velocity.z;
    effect->unknown94 = 0.0f;
    effect->unknown98 = 0.0f;
    effect->unknown9c = 1.0f;
    effect->unknown8c = 0x20;
    effect->unknown7c = 16.0f;
    effect->unknownC9 = 1;
    effect->unknown80 = -1.5707964f;
    return 0;
}
