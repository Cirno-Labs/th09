#include "screeneffect.h"
#include "gamemanager.h"
#include "mainmenu.h"
#include "sprite.h"
#include "unsorted.h"

void ClearViewport(DWORD color)
{
    if (g_SpriteEngine != NULL)
    {
        g_SpriteEngine->FlushQuadBatch();
    }
    g_App.SetView(2);
    ClearScreen(color);
}

int ScreenEffect::OnUpdateFadeIn(ScreenEffect *effect)
{
    int duration = effect->duration;
    if (duration != 0)
    {
        effect->alpha = 255.0f - effect->timer.AsFloat() * 255.0f / duration;
        if (effect->alpha < 0)
        {
            effect->alpha = 0;
        }
    }
    if (effect->timer >= duration)
    {
        return 0;
    }
    effect->timer++;
    return 1;
}

void DrawColoredLineStrip(VertexDiffuseXyzrwh *vertices, int count)
{
    g_SpriteEngine->FlushQuadBatch();
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, count, vertices, sizeof(VertexDiffuseXyzrwh));
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearSpriteData();
    g_SpriteEngine->ClearTexture();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
}

int ScreenEffect::OnUpdateFadeOut(ScreenEffect *effect)
{
    if (g_Unknown4ac884 != 0)
    {
        return 0;
    }
    int duration = effect->duration;
    if (duration != 0)
    {
        effect->alpha = effect->timer.AsFloat() * 255.0f / duration;
        if (effect->alpha < 0)
        {
            effect->alpha = 0;
        }
    }
    if (effect->timer >= duration)
    {
        return 0;
    }
    if (!g_GameManager.isPaused && !g_GameManager.isInContinueMenu)
    {
        effect->timer++;
    }
    return 1;
}

int ScreenEffect::OnUpdateLoadingFade(ScreenEffect *effect)
{
    if (!effect->finishing)
    {
        int duration = effect->duration;
        if (duration != 0 && effect->timer <= duration)
        {
            effect->alpha = effect->timer.AsFloat() * 255.0f / duration;
        }
        else
        {
            effect->alpha = 0xff;
        }
    }
    else if (effect->timer <= 8)
    {
        effect->alpha = 0x80 - (int)(effect->timer.AsFloat() * 255.0f / 8.0f);
    }
    else
    {
        return 0;
    }
    effect->timer++;
    return 1;
}

int ScreenEffect::OnUpdateFlash(ScreenEffect *effect)
{
    DWORD startAlpha = (effect->flashColor >> 24) & 0xff;

    if (g_Unknown4ac884 != 0)
    {
        return 0;
    }
    int duration = effect->duration;
    if (effect->timer < duration)
    {
        effect->alpha = startAlpha - (int)(effect->timer.AsFloat() * startAlpha / duration);
        if (effect->alpha < 0)
        {
            effect->alpha = 0;
        }
    }
    else
    {
        effect->alpha = 0;
        effect->flashes--;
        if (effect->flashes <= 0)
        {
            return 0;
        }
        effect->timer = 0;
    }
    effect->timer++;
    return 1;
}

int ScreenEffect::OnUpdateShake(ScreenEffect *effect)
{
    float shake;

    if (g_GameManager.sides[effect->side].flag0)
    {
        return 1;
    }
    if (g_GameManager.stateFlags & 0x1800)
    {
        return 1;
    }
    if (g_Unknown4ac884 != 0)
    {
        return 0;
    }
    effect->timer++;
    int duration = effect->duration;
    if (effect->timer >= duration)
    {
        return 0;
    }
    int shakeFrom = effect->shakeFrom;
    shake = effect->timer.AsFloat() * (effect->shakeTo - shakeFrom);
    shake = shake / duration;
    shake = shakeFrom + shake;
    switch (g_Rng.GetRandomU32InRange(3))
    {
    case 0:
        g_App.cameras[effect->side].shakeX = 0.0f;
        break;
    case 1:
        g_App.cameras[effect->side].shakeX = shake;
        break;
    case 2:
        g_App.cameras[effect->side].shakeX = -shake;
        break;
    }
    switch (g_Rng.GetRandomU32InRange(3))
    {
    case 0:
        g_App.cameras[effect->side].shakeY = 0.0f;
        break;
    case 1:
        g_App.cameras[effect->side].shakeY = shake;
        break;
    case 2:
        g_App.cameras[effect->side].shakeY = -shake;
        break;
    }
    return 1;
}

int ScreenEffect::DeletedCallback(ScreenEffect *effect)
{
    effect->calcElem->deletedCallback = NULL;
    g_Chain.Remove(effect->drawElem);
    effect->drawElem = NULL;
    delete effect;
    return 0;
}

int ScreenEffect::OnDrawScreen(ScreenEffect *effect)
{
    float area[4];

    area[0] = 0.0f;
    area[1] = 0.0f;
    area[2] = 640.0f;
    area[3] = 480.0f;
    g_SpriteEngine->FlushQuadBatch();
    g_App.SetView(2);
    DrawColorRect(area, effect->alpha << 24 | effect->color);
    return 1;
}

int ScreenEffect::OnDrawPlayArea(ScreenEffect *effect)
{
    float area[4];

    area[0] = 32.0f;
    area[1] = 16.0f;
    area[2] = 416.0f;
    area[3] = 464.0f;
    DrawColorRect(area, effect->alpha << 24 | effect->color);
    return 1;
}

int ScreenEffect::OnDrawWholeScreen(ScreenEffect *effect)
{
    float area[4];

    area[0] = 0.0f;
    area[1] = 0.0f;
    area[2] = 640.0f;
    area[3] = 480.0f;
    DrawColorRect(area, effect->alpha << 24 | effect->color);
    return 1;
}

int ScreenEffect::OnDrawFlash(ScreenEffect *effect)
{
    float area[4];

    area[0] = 32.0f;
    area[1] = 16.0f;
    area[2] = 416.0f;
    area[3] = 464.0f;
    DrawColorRect(area, effect->alpha << 24 | (effect->flashColor & 0xffffff));
    return 1;
}

void ScreenEffect::Finish()
{
    this->finishing = 1;
    this->timer = 0;
}
