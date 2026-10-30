// cflags: -/GX
// The effect object is built without /GX: new EffectManager() has no
// exception frame to free the block if the constructor throws.
#include "effect.h"
#include "gamemanager.h"
#include "player.h"
#include "unsorted.h"

int EffectUpdateFollowPlayer(Effect *effect)
{
    effect->pos = effect->mgr->side->player->pos;
    return 1;
}

void EffectManager::Reset()
{
    int i;
    Effect *effect;

    this->unknown3c.next = NULL;
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

EffectManager *EffectManager::RegisterChain(int sideIdx, int count34, int count38)
{
    EffectManager *mgr =
        (EffectManager *)g_MemoryManager.Register(new EffectManager(), sizeof(EffectManager), "EffectSysInf");

    mgr->Init();
    mgr->sideIdx = sideIdx;
    if (sideIdx < 2)
    {
        mgr->side = &g_GameManager.sides[sideIdx];
        mgr->otherSide = &g_GameManager.sides[1 - sideIdx];
    }
    else
    {
        mgr->side = &g_GameManager.sides[sideIdx];
        mgr->otherSide = &g_GameManager.sides[sideIdx];
    }
    mgr->unknown34 = count34;
    mgr->unknown38 = count38;
    mgr->effects = (Effect *)g_MemoryManager.Alloc((count34 + count38 + 1) * sizeof(Effect), GLOBAL_H);
    memset(mgr->effects, 0, (count34 + count38 + 1) * sizeof(Effect));
    mgr->calcElem = g_Chain.CreateElem((ChainCallback)EffectManager::OnUpdate);
    mgr->calcElem->arg = mgr;
    mgr->calcElem->addedCallback = (ChainCallback)EffectManager::AddedCallback;
    if (g_Chain.AddToCalcChain(mgr->calcElem, sideIdx + 0x13) != 0)
    {
        return NULL;
    }
    if (sideIdx < 2)
    {
        mgr->drawElem = g_Chain.CreateElem((ChainCallback)EffectManager::OnDraw);
        mgr->drawElem->arg = mgr;
        g_Chain.AddToDrawChain(mgr->drawElem, sideIdx + 0x13);
    }
    else
    {
        mgr->drawElem = g_Chain.CreateElem((ChainCallback)EffectManager::OnDraw2);
        mgr->drawElem->arg = mgr;
        g_Chain.AddToDrawChain(mgr->drawElem, 0x1e);
    }
    return mgr;
}

int EffectUpdate40e080(Effect *effect)
{
    D3DXVECTOR3 offset;

    ((Vector2 *)&offset)->SetPolar(effect->angle, 256.0f);
    effect->pos.x += offset.x;
    effect->pos.y += offset.y;
    effect->unknown04[2] = AddNormalizeAngle(effect->angle, 1.5707964f);
    return 0;
}
