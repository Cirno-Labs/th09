// cflags: -/GX
// The effect object is built without /GX: new EffectManager() has no
// exception frame to free the block if the constructor throws.
#include "effect.h"
#include "gamemanager.h"
#include "player.h"
#include "sprite.h"
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
        if (effect->sprites != NULL)
        {
            g_MemoryManager.Free(effect->sprites);
            effect->unknownCc = NULL;
        }
    }
    g_MemoryManager.Free(this->effects);
    this->effects = (Effect *)g_MemoryManager.Alloc(
        (this->unknown34 + this->unknown38 + 1) * sizeof(Effect), GLOBAL_H);
    memset(this->effects, 0, (this->unknown34 + this->unknown38 + 1) * sizeof(Effect));
}

Effect *EffectManager::StartEffect(Effect *effect, int effectIdx, D3DXVECTOR3 *pos, D3DCOLOR color,
                                   D3DXVECTOR3 *velocity)
{
    int i;
    int script;

    if (effect->unknownCc != NULL)
    {
        g_MemoryManager.Free(effect->unknownCc);
    }
    if (effect->sprites != NULL)
    {
        g_MemoryManager.Free(effect->sprites);
    }
    memset(effect, 0, sizeof(Effect));
    effect->type = effectIdx;
    effect->active = 1;
    effect->pos = *pos;
    effect->velocity = *velocity;
    effect->spriteCount = g_EffectInfos[effectIdx].spriteCount;
    effect->mgr = this;
    if (effect->spriteCount != 0)
    {
        effect->sprites = (Sprite *)g_MemoryManager.Alloc(g_EffectInfos[effectIdx].spriteCount * sizeof(Sprite), GLOBAL_H);
        script = g_EffectInfos[effectIdx].script;
        for (i = 0; i < g_EffectInfos[effectIdx].spriteCount; i++)
        {
            this->anm->StartScript(&effect->sprites[i], script);
            script += g_EffectInfos[effectIdx].scriptStep;
            effect->sprites[i].flags |= 0x2000;
            effect->sprites[i].color = color;
            effect->sprites[i].posOffset.x = 0.0f;
            effect->sprites[i].posOffset.y = 0.0f;
            effect->sprites[i].posOffset.z = 0.0f;
        }
    }
    effect->update = g_EffectInfos[effectIdx].update;
    effect->draw = g_EffectInfos[effectIdx].draw;
    if (g_EffectInfos[effectIdx].init != NULL && g_EffectInfos[effectIdx].init(effect) != 0)
    {
        effect->active = 0;
    }
    return effect;
}

Effect *EffectManager::SpawnParticles(int effectIdx, D3DXVECTOR3 *pos, int count, D3DCOLOR color)
{
    Effect *effect = &this->effects[this->next];
    D3DXVECTOR3 velocity(0.0f, 0.0f, 0.0f);
    int tries;

    for (tries = 0; tries < this->unknown34; tries++)
    {
        this->next++;
        if (this->next >= this->unknown34)
        {
            this->next = 0;
        }
        if (!effect->active)
        {
            this->StartEffect(effect, effectIdx, pos, color, &velocity);
            count--;
            if (count == 0)
            {
                break;
            }
        }
        if (this->next == 0)
        {
            effect = this->effects;
        }
        else
        {
            effect++;
        }
    }
    return tries >= this->unknown34 ? &this->effects[this->unknown38 + this->unknown34] : effect;
}

Effect *EffectManager::SpawnParticlesWithVelocity(int effectIdx, D3DXVECTOR3 *pos, D3DXVECTOR3 *velocity, int count,
                                                  D3DCOLOR color)
{
    Effect *effect = &this->effects[this->next];
    int tries;

    for (tries = 0; tries < this->unknown34; tries++)
    {
        this->next++;
        if (this->next >= this->unknown34)
        {
            this->next = 0;
        }
        if (!effect->active)
        {
            this->StartEffect(effect, effectIdx, pos, color, velocity);
            count--;
            if (count == 0)
            {
                break;
            }
        }
        if (this->next == 0)
        {
            effect = this->effects;
        }
        else
        {
            effect++;
        }
    }
    return tries >= this->unknown34 ? &this->effects[this->unknown38 + this->unknown34] : effect;
}

Effect *EffectManager::SpawnReservedEffect(int effectIdx, D3DXVECTOR3 *pos, int slot, D3DCOLOR color)
{
    Effect *effect = &this->effects[this->unknown34 + slot];
    D3DXVECTOR3 velocity(0.0f, 0.0f, 0.0f);

    this->StartEffect(effect, effectIdx, pos, color, &velocity);
    effect->reservedSlot = slot;
    return effect;
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
        if (effect->sprites != NULL)
        {
            g_MemoryManager.Free(effect->sprites);
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

int EffectInit40e080(Effect *effect)
{
    D3DXVECTOR3 offset;

    ((Vector2 *)&offset)->SetPolar(effect->velocity.x, 256.0f);
    effect->pos.x += offset.x;
    effect->pos.y += offset.y;
    effect->sprites->rotation.z = AddNormalizeAngle(effect->velocity.x, 1.5707964f);
    return 0;
}
