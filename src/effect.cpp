// cflags: -/GX
// The effect object is built without /GX: new EffectManager() has no
// exception frame to free the block if the constructor throws.
#include "effect.h"
#include "enemy.h"
#include "gamemanager.h"
#include "gui.h"
#include "player.h"
#include "sound.h"
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

    this->drawList0.next = NULL;
    effect = this->effects;
    for (i = 0; i < this->unknown34 + this->unknown38 + 1; i++, effect++)
    {
        if (effect->vertices != NULL)
        {
            g_MemoryTracker.Free(effect->vertices);
            effect->vertices = NULL;
        }
        if (effect->sprites != NULL)
        {
            g_MemoryTracker.Free(effect->sprites);
            effect->vertices = NULL;
        }
    }
    g_MemoryTracker.Free(this->effects);
    this->effects = (Effect *)g_MemoryTracker.Alloc(
        (this->unknown34 + this->unknown38 + 1) * sizeof(Effect), GLOBAL_H);
    memset(this->effects, 0, (this->unknown34 + this->unknown38 + 1) * sizeof(Effect));
}

Effect *EffectManager::StartEffect(Effect *effect, int effectIdx, D3DXVECTOR3 *pos, D3DCOLOR color,
                                   D3DXVECTOR3 *velocity)
{
    int i;
    int script;

    if (effect->vertices != NULL)
    {
        g_MemoryTracker.Free(effect->vertices);
    }
    if (effect->sprites != NULL)
    {
        g_MemoryTracker.Free(effect->sprites);
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
        effect->sprites = (Sprite *)g_MemoryTracker.Alloc(g_EffectInfos[effectIdx].spriteCount * sizeof(Sprite), GLOBAL_H);
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

Effect *EffectManager::SpawnEffect(int effectIdx, D3DXVECTOR3 *pos, int count, D3DCOLOR color)
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

Effect *EffectManager::SpawnEffectWithVelocity(int effectIdx, D3DXVECTOR3 *pos, D3DXVECTOR3 *velocity, int count,
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

int EffectManager::OnUpdate(EffectManager *mgr)
{
    Effect *effect = mgr->effects;
    int i;
    int j;
    int running;

    if (g_GameManager.stateFlags & 0x1800)
    {
        return 1;
    }
    if (g_GameManager.sides[0].flag0)
    {
        return 1;
    }
    g_App.SetCamera(mgr->sideIdx);
    mgr->activeCount = 0;
    mgr->drawListTails[0] = &mgr->drawList0;
    mgr->drawListTails[1] = &mgr->drawList1;
    mgr->drawListTails[2] = &mgr->drawList2;
    mgr->drawList0.next = NULL;
    mgr->drawList1.next = NULL;
    mgr->drawList2.next = NULL;
    // Preserve the original order of the two live pool-bound reads.
    for (i = 0; i < static_cast<const volatile int &>(mgr->unknown34) + mgr->unknown38; i++, effect++)
    {
        if (!effect->active)
        {
            if (effect->vertices != NULL)
            {
                g_MemoryTracker.Free(effect->vertices);
                effect->vertices = NULL;
            }
            if (effect->sprites != NULL)
            {
                g_MemoryTracker.Free(effect->sprites);
                effect->sprites = NULL;
            }
            continue;
        }
        mgr->activeCount++;
        if (effect->update != NULL && effect->update(effect) != 1)
        {
            effect->active = 0;
            continue;
        }
        running = effect->spriteCount;
        for (j = 0; j < effect->spriteCount; j++)
        {
            if (g_SpriteEngine->Step(&effect->sprites[j]) != 0)
            {
                running--;
            }
        }
        if (running == 0)
        {
            effect->active = 0;
            continue;
        }
        effect->timer++;
        effect->next = NULL;
        if (effect->unknownD0)
        {
            continue;
        }
        if (effect->unknownCa)
        {
            mgr->drawListTails[2]->next = effect;
            mgr->drawListTails[2] = effect;
        }
        else if (effect->unknownC8)
        {
            mgr->drawListTails[1]->next = effect;
            mgr->drawListTails[1] = effect;
        }
        else
        {
            mgr->drawListTails[0]->next = effect;
            mgr->drawListTails[0] = effect;
        }
    }
    mgr->frames++;
    return 1;
}

int EffectManager::OnDraw(EffectManager *mgr)
{
    Effect *effect;
    Sprite *sprite;
    int i;

    effect = mgr->drawList0.next;
    g_App.SetView(mgr->sideIdx);
    for (; effect != NULL; effect = effect->next)
    {
        if (effect->draw != NULL)
        {
            effect->draw(effect);
        }
        else
        {
            sprite = effect->sprites;
            for (i = effect->spriteCount; i > 0; i--, sprite++)
            {
                sprite->pos.x = g_GameManager.ArenaToScreenX(effect->pos.x);
                sprite->pos.y = g_GameManager.ArenaToScreenY(effect->pos.y);
                sprite->pos.z = 0.08f;
                sprite->pos += sprite->posOffset;
                g_SpriteEngine->Draw(sprite);
            }
        }
    }
    return 1;
}

int EffectManager::OnDraw2(EffectManager *mgr)
{
    Effect *effect;
    Sprite *sprite;
    int i;

    effect = mgr->drawList0.next;
    g_App.SetView(mgr->sideIdx);
    for (; effect != NULL; effect = effect->next)
    {
        if (effect->draw != NULL)
        {
            effect->draw(effect);
        }
        else
        {
            sprite = effect->sprites;
            for (i = effect->spriteCount; i > 0; i--, sprite++)
            {
                sprite->pos.x = effect->pos.x;
                sprite->pos.y = effect->pos.y;
                sprite->pos.z = 0.0f;
                sprite->pos += sprite->posOffset;
                g_SpriteEngine->Draw(sprite);
            }
        }
    }
    return 1;
}

int EffectManager::DrawUnknown114(EffectManager *mgr)
{
    Effect *effect;
    Sprite *sprite;
    int i;

    for (effect = mgr->drawList1.next; effect != NULL; effect = effect->next)
    {
        if (effect->draw != NULL)
        {
            effect->draw(effect);
        }
        else
        {
            sprite = effect->sprites;
            for (i = effect->spriteCount; i > 0; i--, sprite++)
            {
                sprite->pos.x = g_GameManager.ArenaToScreenX(effect->pos.x);
                sprite->pos.y = g_GameManager.ArenaToScreenY(effect->pos.y);
                sprite->pos.z = 0.08f;
                sprite->pos += sprite->posOffset;
                g_SpriteEngine->Draw(sprite);
            }
        }
    }
    return 1;
}

int EffectManager::AddedCallback(EffectManager *mgr)
{
    AnmLoaded *anm = g_SpriteEngine->GetAnm(8);

    if (anm->file == NULL)
    {
        anm = g_SpriteEngine->PreloadAnm(8, "etama.anm");
    }
    mgr->anm = anm;
    return 0;
}

int EffectManager::FreeEffects(EffectManager *mgr)
{
    int i;
    Effect *effect;

    effect = mgr->effects;
    for (i = 0; i < mgr->unknown38 + mgr->unknown34 + 1; i++, effect++)
    {
        if (effect->vertices != NULL)
        {
            g_MemoryTracker.Free(effect->vertices);
            effect->vertices = NULL;
        }
        if (effect->sprites != NULL)
        {
            g_MemoryTracker.Free(effect->sprites);
            effect->vertices = NULL;
        }
    }
    g_MemoryTracker.Free(mgr->effects);
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
        (EffectManager *)g_MemoryTracker.Register(new EffectManager(), sizeof(EffectManager), "EffectSysInf");

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
    mgr->effects = (Effect *)g_MemoryTracker.Alloc((count34 + count38 + 1) * sizeof(Effect), GLOBAL_H);
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

int UpdateEffectFromSprite(Effect *effect)
{
    effect->unknown8c = effect->sprites->scriptVar0;
    effect->unknown9c = effect->sprites->scriptVar1;
    effect->unknown88 = effect->sprites->scaleX;
    effect->unknown7c = effect->sprites->posOffset.x;
    effect->unknown94 = effect->sprites->posOffset.y;
    effect->unknown80 = effect->sprites->rotation.z;
    effect->unknown98 = effect->sprites->rotation.y;
    effect->unknownC9 = 1;
    return 1;
}

void Vector2::SetPolarEllipse(float angle, float radiusX, float radiusY)
{
    __asm
    {
        mov eax, this
        fld angle
        fsincos
        fmul radiusX
        fstp [eax]
        fmul radiusY
        fstp [eax+4]
    }
}

int InitEffectTypes1And2(Effect *effect)
{
    effect->unknown54 = effect->pos;
    effect->unknown60.x = g_GameManager.ArenaToScreenX(effect->velocity.x);
    effect->unknown60.y = g_GameManager.ArenaToScreenY(effect->velocity.y);
    effect->unknown60.z = effect->velocity.z;
    SetPolar((Vector2 *)&effect->unknown3c,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(256.0f) + 64.0f);
    SetPolar((Vector2 *)&effect->unknown48,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(256.0f) + 64.0f);
    effect->unknown9c = g_Rng.GetRandomF32InRange(100.0f) + 60.0f;
    effect->unknown98 = 0.0f;
    return 0;
}

int UpdateEffectTypes1And2(Effect *effect)
{
    float t;
    float f00;
    float f01;
    float f10;
    float f11;
    float maxSpeed;

    if (effect->unknown98 != 0.0f)
    {
        effect->unknownD0 = 1;
        if (effect->timer.AsFloat() >= effect->unknown98)
        {
            g_SoundPlayer.QueueSound(0x2e, effect->mgr->sideIdx != 0 ? 500 : -500);
            effect->unknown98 = 0.0f;
            effect->timer = 0;
            effect->unknownD0 = 0;
            return 1;
        }
        return 1;
    }
    if (effect->timer.AsFloat() < effect->unknown9c)
    {
        t = effect->timer.AsFloat() / effect->unknown9c;
        f00 = (2.0f * t + 1.0f) * (t - 1.0f) * (t - 1.0f);
        f01 = (3.0f - 2.0f * t) * t * t;
        f10 = (1.0f - t) * (1.0f - t) * t;
        f11 = (t - 1.0f) * t * t;
        effect->pos.x = f00 * effect->unknown54.x + f10 * effect->unknown3c.x + f01 * effect->unknown60.x +
                        f11 * effect->unknown48.x;
        effect->pos.y = f00 * effect->unknown54.y + f10 * effect->unknown3c.y + f01 * effect->unknown60.y +
                        f11 * effect->unknown48.y;
        return 1;
    }
    if (g_GameManager.gui->GetUnknown11ea8() != 0)
    {
        return 0;
    }
    BulletShooter shooter;
    shooter.pos = effect->velocity;
    if (effect->unknownAa >= 2 && g_Rng.GetRandomU32InRange(5) == 0)
    {
        shooter.sprite = 3;
    }
    else
    {
        shooter.sprite = effect->unknownA4;
    }
    shooter.color = effect->unknownA6;
    shooter.aimMode = 1;
    shooter.count1 = 1;
    shooter.count2 = 1;
    if (shooter.sprite == 3)
    {
        shooter.angle1 = g_Rng.GetRandomF32SignedInRange(0.1308997f) + 1.5707964f;
    }
    else if (g_Rng.GetRandomU32InRange(3))
    {
        shooter.color = 5;
        shooter.aimMode = 0;
        shooter.angle1 = g_Rng.GetRandomF32SignedInRange(0.22439948f);
    }
    else
    {
        shooter.color = 3;
        shooter.angle1 = g_Rng.GetRandomF32SignedInRange(0.2617994f) + 1.5707964f;
    }
    shooter.speed1 = g_Rng.GetRandomF32SignedInRange(0.4f) + effect->unknownA0;
    if (g_GameManager.sides[1 - effect->unknownA8].character == 10)
    {
        shooter.speed1 *= 0.93f;
    }
    if (shooter.sprite == 3)
    {
        shooter.speed1 += 0.3f;
    }
    maxSpeed = g_GameManager.rank.unknownB4 * 0.1f + 3.0f;
    if (shooter.speed1 >= maxSpeed)
    {
        shooter.speed1 = maxSpeed;
    }
    else if (shooter.speed1 < 0.8f)
    {
        shooter.speed1 = 0.8f;
    }
    shooter.unknown20c = effect->unknownAa + 1;
    shooter.angle2 = 0.0f;
    shooter.speed2 = 0.0f;
    shooter.unknown1fa = 0;
    shooter.flags = 4;
    g_GameManager.sides[1 - effect->unknownA8].bulletManager->FirePattern(&shooter);
    return 0;
}

int InitEffectTypes3And4(Effect *effect)
{
    effect->unknown54 = effect->pos;
    effect->unknown60.x = g_GameManager.ArenaToScreenX(effect->velocity.x);
    effect->unknown60.y = g_GameManager.ArenaToScreenY(effect->velocity.y);
    effect->unknown60.z = effect->velocity.z;
    SetPolar((Vector2 *)&effect->unknown3c,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(256.0f) + 64.0f);
    SetPolar((Vector2 *)&effect->unknown48,
             g_Rng.GetRandomF32SignedInRange(3.1415927f), g_Rng.GetRandomF32InRange(256.0f) + 64.0f);
    effect->unknown9c = g_Rng.GetRandomF32InRange(100.0f) + 60.0f;
    return 0;
}

int UpdateEffectTypes3And4(Effect *effect)
{
    float t;
    float f00;
    float f01;
    float f10;
    float f11;
    Enemy *enemy;
    int range;

    if (effect->timer.AsFloat() < effect->unknown9c)
    {
        t = effect->timer.AsFloat() / effect->unknown9c;
        f00 = (2.0f * t + 1.0f) * (t - 1.0f) * (t - 1.0f);
        f01 = (3.0f - 2.0f * t) * t * t;
        f10 = (1.0f - t) * (1.0f - t) * t;
        f11 = (t - 1.0f) * t * t;
        effect->pos.x = f00 * effect->unknown54.x + f10 * effect->unknown3c.x + f01 * effect->unknown60.x +
                        f11 * effect->unknown48.x;
        effect->pos.y = f00 * effect->unknown54.y + f10 * effect->unknown3c.y + f01 * effect->unknown60.y +
                        f11 * effect->unknown48.y;
        return 1;
    }
    if (g_GameManager.gui->GetUnknown11ea8() == 0 &&
        g_GameManager.sides[1 - effect->unknownA8].enemyManager->unknown2ac3b8 < 10)
    {
        enemy = g_GameManager.sides[1 - effect->unknownA8].enemyManager->Spawn(
            effect->unknownA4, &effect->velocity, 0x14, -2, 1000, 0, 0);
        enemy->unknown3380 = effect->unknownA6;
        enemy->acceleration += effect->unknownA0;
        range = g_GameManager.sides[1 - effect->unknownA8].character != 7 ? 5 : 4;
        if (g_Rng.GetRandomU32InRange(range))
        {
            enemy->angle = g_Rng.GetRandomF32SignedInRange(0.015707964f) + 1.5707964f;
            return 0;
        }
        enemy->angle = g_GameManager.sides[1 - effect->unknownA8].player->AngleToPlayer(&enemy->absPos) +
                       g_Rng.GetRandomF32SignedInRange(0.3926991f);
    }
    return 0;
}

int EffectInitRing(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryTracker.Alloc(0x1c38, "Effect");
    if (effect->vertices == NULL)
    {
        return -1;
    }
    effect->unknown48 = effect->pos;
    effect->unknown80 = effect->velocity.x;
    effect->unknown8c = 3;
    effect->unknown54.x = 0.0f;
    effect->unknown54.y = 0.0f;
    effect->unknown54.z = 1.0f;
    effect->unknown60.x = 0.0f;
    effect->unknown60.y = -1.0f;
    effect->unknown60.z = 0.0f;
    effect->unknown7c = effect->velocity.y;
    effect->unknown88 = effect->velocity.z;
    g_SpriteEngine->FillSpriteStripVertices(effect->sprites, effect->vertices, 6);
    effect->unknown94 = 0.0f;
    effect->unknown98 = 0.0f;
    effect->unknown9c = 0.0f;
    effect->unknownC9 = 1;
    effect->unknown8c = 0x18;
    return 0;
}

int EffectDrawRing(Effect *effect)
{
    float outer;
    float delta;
    VertexTex1DiffuseXyzrwh *vtx;
    float inner;
    int i;

    if (effect->unknownC9)
    {
        delta = 6.2831855f / effect->unknown8c;
        outer = effect->unknown88 / sinf((3.1415927f - delta) / 2.0f);
        vtx = effect->vertices;
        g_SpriteEngine->FillVerticalStripVertices(effect->sprites, effect->vertices, effect->unknown8c * 2 + 2);
        if (effect->unknown94 == 0.0f)
        {
            float angle = effect->unknown80;

            inner = effect->unknown7c - outer;
            outer += effect->unknown7c;
            for (i = effect->unknown8c + 1; i > 0; i--)
            {
                if (angle >= 3.1415927f)
                {
                    angle -= 6.2831855f;
                }
                vtx->pos.z = 0.0f;
                SetPolar((Vector2 *)&vtx->pos, angle, outer);
                vtx->pos += effect->unknown48;
                vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
                vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
                vtx++;
                vtx->pos.z = 0.0f;
                SetPolar((Vector2 *)&vtx->pos, angle, inner);
                vtx->pos += effect->unknown48;
                vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
                vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
                vtx++;
                angle += delta;
            }
        }
        else if (effect->unknown9c == 0.0f)
        {
            float angle = 0.0f;
            D3DXVECTOR3 pos;
            Vector2 *ppos = (Vector2 *)&pos;
            float outerY;
            float ryInner;

            outerY = outer + effect->unknown94;
            ryInner = effect->unknown94 - outer;
            inner = effect->unknown7c - outer;
            outer += effect->unknown7c;
            for (i = effect->unknown8c + 1; i > 0; i--)
            {
                ppos->SetPolarEllipse(angle, outer, outerY);
                Rotate2D(&vtx->pos, (D3DXVECTOR3 *)ppos, effect->unknown98);
                vtx->pos += effect->unknown48;
                vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
                vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
                vtx->pos.z = 0.0f;
                vtx++;
                ppos->SetPolarEllipse(angle, inner, ryInner);
                Rotate2D(&vtx->pos, (D3DXVECTOR3 *)ppos, effect->unknown98);
                vtx->pos += effect->unknown48;
                vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
                vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
                vtx->pos.z = 0.0f;
                vtx++;
                angle += delta;
            }
        }
        else
        {
            float wobbleAngle = effect->unknown98;
            float angle = effect->unknown80;
            float phaseStep = 6.2831855f * effect->unknown9c / effect->unknown8c;
            float wobble;

            inner = effect->unknown7c - outer;
            outer += effect->unknown7c;
            for (i = effect->unknown8c + 1; i > 0; i--)
            {
                if (angle >= 3.1415927f)
                {
                    angle -= 6.2831855f;
                }
                if (wobbleAngle >= 3.1415927f)
                {
                    wobbleAngle -= 6.2831855f;
                }
                wobble = cosf(wobbleAngle) * effect->unknown94;
                SetPolar((Vector2 *)&vtx->pos, angle, outer + wobble);
                vtx->pos += effect->unknown48;
                vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
                vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
                vtx->pos.z = 0.0f;
                vtx++;
                SetPolar((Vector2 *)&vtx->pos, angle, inner + wobble);
                vtx->pos += effect->unknown48;
                vtx->pos.x = g_GameManager.ArenaToScreenX(vtx->pos.x);
                vtx->pos.y = g_GameManager.ArenaToScreenY(vtx->pos.y);
                vtx->pos.z = 0.0f;
                vtx++;
                angle += delta;
                wobbleAngle += phaseStep;
            }
        }
        effect->unknownC9 = 0;
    }
    g_SpriteEngine->DrawVertexStrip(effect->sprites, effect->vertices, effect->unknown8c * 2 + 2);
    return 1;
}

int InitOrientedEffect(Effect *effect)
{
    D3DXVECTOR3 offset;

    SetPolar((Vector2 *)&offset, effect->velocity.x, 256.0f);
    effect->pos.x += offset.x;
    effect->pos.y += offset.y;
    effect->sprites->rotation.z = AddNormalizeAngle(effect->velocity.x, 1.5707964f);
    return 0;
}
