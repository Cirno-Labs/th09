#include "effect.h"

int EffectManager::FreeEffects(EffectManager *mgr)
{
    int i;
    Effect *effect;

    effect = mgr->effects;
    for (i = 0; i < mgr->unknown38 + mgr->unknown34 + 1; i++, effect++)
    {
        if (effect->unknownCc != NULL)
        {
            g_MemoryManager.Free(effect->unknownCc);
            effect->unknownCc = NULL;
        }
        if (effect->unknown04 != NULL)
        {
            g_MemoryManager.Free(effect->unknown04);
            effect->unknownCc = NULL;
        }
    }
    g_MemoryManager.Free(mgr->effects);
    return 0;
}
