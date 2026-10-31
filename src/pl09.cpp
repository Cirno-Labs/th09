// Character 9's extra attacks and callbacks (pl/pl09.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// The unknown30450 of character 9: while unknown34 reaches (30 -
// unknownB4) * 2, sends an extra attack of type 17 from `pos` and
// takes as much off it. Returns 0 (0x448640).
int PlayerComboFunc448640(PlayerUnknown30410 *combo, D3DXVECTOR3 *pos)
{
    while (combo->unknown34 >= (30 - g_GameManager.unknownB4) * 2)
    {
        g_GameManager.exAttackManager->AddAttack(17, pos, combo->player->sideIdx, NULL);
        combo->unknown34 -= (30 - g_GameManager.unknownB4) * 2;
    }
    return 0;
}

// An init callback of the effect table: allocates 0xf78 bytes of
// vertices and sets the effect up as a ring of 32 segments, 32 out,
// around its position, as wide as velocity.z. Returns 0, or -1 if the
// allocation fails (0x4486b0).
int EffectInit4486b0(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryManager.Alloc(0xf78, "Effect");
    if (effect->vertices == NULL)
    {
        return -1;
    }
    effect->unknown48 = effect->pos;
    effect->unknown54.z = 1.0f;
    effect->unknown9c = 1.0f;
    effect->unknown54.x = 0.0f;
    effect->unknown54.y = 0.0f;
    effect->unknown60.x = 0.0f;
    effect->unknown60.y = -1.0f;
    effect->unknown60.z = 0.0f;
    effect->unknown80 = 0.0f;
    effect->unknown88 = effect->velocity.z;
    effect->unknown94 = 0.0f;
    effect->unknown98 = 0.0f;
    effect->unknown8c = 0x20;
    effect->unknown7c = 32.0f;
    effect->unknownC9 = 1;
    return 0;
}

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
