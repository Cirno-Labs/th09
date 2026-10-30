#include "stage.h"
#include "gamemanager.h"
#include "screeneffect.h"
#include "unsorted.h"

// Scales color channel `c` by `factor`, where 0x80 is 1, clamping to
// 0xff. Inline in th08; static here, taking its arguments in al and cl
// (0x401090).
static BYTE ModulateChannel(BYTE c, BYTE factor)
{
    DWORD channel = (DWORD)c * factor >> 7;
    if (channel >= 0x100)
    {
        channel = 0xff;
    }
    return channel;
}

// Scales byte `i` of `color` by byte `i` of `factor`.
#define MODULATE_CHANNEL(color, factor, i) \
    ((BYTE *)&(color))[i] = ModulateChannel(((BYTE *)&(color))[i], ((BYTE *)&(factor))[i])

void Stage::Reset()
{
    this->spellCardState = 0;
}

int Stage::DeletedCallback(Stage *stage)
{
    if (FreesResources())
    {
        g_SpriteEngine->ReleaseAnm(4);
    }
    if (stage->unknown24 != NULL)
    {
        g_MemoryManager.Free(stage->unknown24);
        stage->unknown24 = NULL;
    }
    if (stage->unknown820 != NULL)
    {
        g_MemoryManager.Free(stage->unknown820);
        stage->unknown820 = NULL;
    }
    return 0;
}

void Stage::RemoveChain(Stage *stage)
{
    if (stage != NULL)
    {
        g_Chain.Remove(stage->calcElem);
        g_Chain.Remove(stage->drawElem);
        g_Chain.Remove(stage->drawElem2);
        delete stage;
    }
}

int Stage::OnDraw(Stage *stage)
{
    D3DCOLOR color;

    if (!g_App.GetNoFog())
    {
        g_App.DisableFog();
    }
    g_SpriteEngine->FlushQuadBatch();
    g_SpriteEngine->ClearTexture();
    g_App.SetView(stage->sideIdx);
    if (stage->clearScreenOnce != 0)
    {
        g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0);
        stage->clearScreenOnce = 0;
    }
    if (stage->tintColor.a > 0)
    {
        g_SpriteEngine->SetColorModulation(stage->tintColor.color);
    }
    if ((stage->clearColor & 0xff000000) == 0xff000000)
    {
        color = stage->fogColor;
        if (g_SpriteEngine->modulateColorEnabled != 0)
        {
            MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 2);
            MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 1);
            MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 0);
            MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 3);
        }
        g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);
    }
    else if (stage->clearColor != 0)
    {
        float area[4];

        area[0] = 32.0f;
        area[1] = 16.0f;
        area[2] = 416.0f;
        area[3] = 464.0f;
        DrawColorRect(area, stage->fogColor);
        g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, stage->clearColor, 1.0f, 0);
    }
    else
    {
        g_App.d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, stage->clearColor, 1.0f, 0);
    }
    if (stage->spellCardState <= 1)
    {
        if (stage->backgroundLayer0.spriteIndex > 0)
        {
            stage->backgroundLayer0.pos.x = g_GameManager.ArenaToScreenX(-144.0f);
            stage->backgroundLayer0.pos.z = 0.99f;
            g_SpriteEngine->DrawAndFlush(&stage->backgroundLayer0);
        }
        if (stage->backgroundLayer1.spriteIndex > 0)
        {
            g_SpriteEngine->DrawAndFlush(&stage->backgroundLayer1);
        }
    }
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    if (g_SpriteEngine->modulateColorEnabled == 0)
    {
        g_App.SetRenderState(D3DRS_FOGCOLOR, stage->fogColor);
    }
    else
    {
        color = stage->fogColor;
        MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 2);
        MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 1);
        MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 0);
        g_App.SetRenderState(D3DRS_FOGCOLOR, color);
    }
    g_App.SetRenderState(D3DRS_FOGSTART, *(DWORD *)&stage->fogNear);
    g_App.SetRenderState(D3DRS_FOGEND, *(DWORD *)&stage->fogFar);
    if (!g_App.GetNoFog())
    {
        g_App.EnableFog();
    }
    if (stage->spellCardState <= 1)
    {
        stage->DrawLayer(0);
        stage->DrawLayer(1);
    }
    if (stage->keepColorModulation == 0)
    {
        g_SpriteEngine->ResetColorModulation();
    }
    return 1;
}

Stage::Stage()
{
    memset(this, 0, sizeof(Stage));
}

Stage *Stage::RegisterChain(int sideIdx)
{
    Stage *stage = (Stage *)g_MemoryManager.Register(new Stage(), sizeof(Stage), "BGScrollInf");

    stage->unknownC68 = 0;
    stage->sideIdx = sideIdx;
    stage->side = &g_GameManager.sides[sideIdx];
    stage->otherSide = &g_GameManager.sides[1 - sideIdx];
    stage->calcElem = g_Chain.CreateElem((ChainCallback)Stage::OnUpdate);
    stage->calcElem->arg = stage;
    stage->calcElem->addedCallback = (ChainCallback)Stage::AddedCallback;
    stage->calcElem->deletedCallback = (ChainCallback)Stage::DeletedCallback;
    if (g_Chain.AddToCalcChain(stage->calcElem, sideIdx + 8) != 0)
    {
        return NULL;
    }
    stage->drawElem = g_Chain.CreateElem((ChainCallback)Stage::OnDraw);
    stage->drawElem2 = g_Chain.CreateElem((ChainCallback)Stage::OnDraw2);
    stage->drawElem->arg = stage;
    stage->drawElem2->arg = stage;
    g_Chain.AddToDrawChain(stage->drawElem, sideIdx + 6);
    g_Chain.AddToDrawChain(stage->drawElem2, sideIdx + 8);
    return stage;
}
