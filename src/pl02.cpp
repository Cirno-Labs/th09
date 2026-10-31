// Character 2's extra attacks and callbacks (pl/pl02.cpp).

#include "effect.h"
#include "exattack.h"
#include "gamemanager.h"
#include "player.h"

// An update callback of the effect table: after 30 frames, grows
// unknown84 by 0.001 up to 0.08; sets unknownC9 and puts the effect on
// its side's player. Returns 1 (0x441d30).
int EffectUpdate441d30(Effect *effect)
{
    effect->unknownC9 = 1;
    if (effect->unknownAc >= 0x1e)
    {
        if (effect->unknown84 < 0.08f)
        {
            effect->unknown84 += 0.001f;
        }
    }
    effect->pos = g_GameManager.sides[effect->mgr->sideIdx].player->pos;
    return 1;
}

