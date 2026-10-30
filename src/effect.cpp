#include "effect.h"

void EffectManager::Reset()
{
    int i;
    Effect *effect;

    this->unknown110 = 0;
    effect = this->effects;
    for (i = 0; i < this->unknown34 + this->unknown38 + 1; i++, effect++)
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
    g_MemoryManager.Free(this->effects);
    this->effects = (Effect *)g_MemoryManager.Alloc(
        (this->unknown34 + this->unknown38 + 1) * sizeof(Effect), GLOBAL_H);
    memset(this->effects, 0, (this->unknown34 + this->unknown38 + 1) * sizeof(Effect));
}

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

void EffectManager::RemoveChain(EffectManager *mgr)
{
    if (mgr != NULL)
    {
        FreeEffects(mgr);
        g_Chain.Remove(mgr->calcElem);
        g_Chain.Remove(mgr->drawElem);
        delete mgr;
    }
}
