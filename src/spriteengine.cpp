#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "gdisurface.h"
#include "unsorted.h"
#include "sprite.h"

void AnmLoaded::StartScriptAtOrigin(Sprite *vm, int script)
{
    vm->scriptIndex = script;
    vm->pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    vm->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    vm->fontHeight = 15;
    vm->fontWidth = 15;
    this->SetScript(vm, this->scripts[script]);
}

void SpriteEngine::ClearTexture()
{
    this->currentTexture = NULL;
}

void SpriteEngine::SetLastCameraSetupFlag(BYTE value)
{
    this->lastCameraSetupFlag = value;
}

void SpriteEngine::ResetColorModulation()
{
    this->modulateColorEnabled = 0;
    this->modulateColor = 0x80808080;
}

void SpriteEngine::SetColorModulation(D3DCOLOR color)
{
    this->modulateColorEnabled = 1;
    this->modulateColor = color;
}

AnmLoaded *SpriteEngine::GetAnm(int slot)
{
    return &this->anms[slot];
}

void SpriteEngine::ClearBlendMode()
{
    this->blendMode = 3;
}

void SpriteEngine::ClearVertexFormat()
{
    this->vertexFormat = 0xff;
}

void SpriteEngine::ClearSpriteData()
{
    this->currentSpriteData = NULL;
}

void SpriteEngine::ClearColorOp()
{
    this->colorOp = 0xff;
}

void SpriteEngine::ClearZWrite()
{
    this->zWriteDisabled = 0xff;
}

void SpriteEngine::ClearLastCameraSetupFlag()
{
    this->lastCameraSetupFlag = 0xff;
}

void AnmLoaded::SetScript(Sprite *vm, void *script)
{
    if (script == NULL || this->loadStep != 0)
    {
        memset(vm, 0, sizeof(Sprite));
    }
    else
    {
        vm->Initialize();
        vm->anmSlot = this->slot;
        vm->anm = this;
        vm->flags &= ~0x600;
        vm->scriptStart = script;
        vm->scriptPos = vm->scriptStart;
        vm->scriptTime = 0;
        vm->flags &= ~1;
        g_SpriteEngine->Step(vm);
        g_SpriteEngine->scriptStartCount++;
    }
}

void SpriteEngine::RotatePoint(float *out, float x, float y, float sine, float cosine, float cx, float cy)
{
    out[0] = x * cosine - y * sine + cx;
    out[1] = x * sine + y * cosine + cy;
}

void SpriteEngine::ReleaseTexture(AnmTexture *tex)
{
    if (tex->texture != NULL)
    {
        tex->texture->Release();
        tex->texture = NULL;
    }
    if (tex->data != NULL)
    {
        g_MemoryManager.Free(tex->data);
    }
}

void AnmLoaded::LoadSprite(int index, SpriteData *sprite)
{
    this->sprites[index] = *sprite;
    this->sprites[index].uvStartX = this->sprites[index].startX / this->sprites[index].textureWidth;
    this->sprites[index].uvEndX = this->sprites[index].endX / this->sprites[index].textureWidth;
    this->sprites[index].uvStartY = this->sprites[index].startY / this->sprites[index].textureHeight;
    this->sprites[index].uvEndY = this->sprites[index].endY / this->sprites[index].textureHeight;
    this->sprites[index].width = (this->sprites[index].endX - this->sprites[index].startX) / sprite->scaleX;
    this->sprites[index].height = (this->sprites[index].endY - this->sprites[index].startY) / sprite->scaleY;
}

void SpriteEngine::RenderTextToTextureIndex(IDirect3DTexture8 *texture, int x, int y, int width, int unused,
                                            int fontHeight, int destHeight, D3DCOLOR textColor, D3DCOLOR shadowColor,
                                            char *text, float scaleX, float scaleY)
{
    if (fontHeight <= 0)
    {
        fontHeight = 15;
    }
    if (destHeight <= 0)
    {
        destHeight = 15;
    }
    if (fontHeight > 8)
    {
        RenderTextToTextureLarge(x, y, width, unused, fontHeight * scaleX, destHeight * scaleY, textColor,
                                 shadowColor, text, texture);
    }
    else
    {
        RenderTextToTexture(x, y, width, unused, 8, 8, textColor, shadowColor, text, texture);
    }
}

void SpriteEngine::RenderText(Sprite *sprite, D3DCOLOR textColor, D3DCOLOR shadowColor, char *fmt, ...)
{
    va_list args;
    char buf[0x80];
    int fontW;

    fontW = sprite->fontWidth;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    this->RenderTextToTextureIndex(sprite->spriteData->texture, sprite->spriteData->startX,
                                   sprite->spriteData->startY, sprite->spriteData->textureWidth,
                                   sprite->spriteData->textureHeight, fontW, sprite->fontHeight, textColor,
                                   shadowColor, buf, sprite->spriteData->scaleX, sprite->spriteData->scaleY);
    sprite->flags |= 1;
}

void SpriteEngine::RenderTextCentered(Sprite *vm, D3DCOLOR color, D3DCOLOR shadowColor, char *fmt, ...)
{
    va_list args;
    int xPos;
    char buf[0x44];
    int fontW;

    fontW = vm->fontWidth <= 0 ? 15 : vm->fontWidth;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    xPos = vm->spriteData->startX + vm->spriteData->width * vm->spriteData->scaleX / 2.0f -
           (float)strlen(buf) * fontW * vm->spriteData->scaleX / 4.0f;
    this->RenderTextToTextureIndex(vm->spriteData->texture, xPos, vm->spriteData->startY,
                                   vm->spriteData->textureWidth, vm->spriteData->textureHeight, fontW,
                                   vm->fontHeight, color, shadowColor, buf, vm->spriteData->scaleX,
                                   vm->spriteData->scaleY);
    vm->flags |= 1;
}

void SpriteEngine::ReleaseSurface(int surface)
{
    if (this->surfaces[surface] != NULL)
    {
        this->surfaces[surface]->Release();
        this->surfaces[surface] = NULL;
    }
    if (this->surfacesBackup[surface] != NULL)
    {
        this->surfacesBackup[surface]->Release();
        this->surfacesBackup[surface] = NULL;
    }
    if (this->surfaceFiles[surface] != NULL)
    {
        g_MemoryManager.Free(this->surfaceFiles[surface]);
    }
    this->surfaceFiles[surface] = NULL;
}

void SpriteEngine::CopySurfaceToBackBuffer(int surface, int left, int top, int x, int y)
{
    IDirect3DSurface8 *backBuffer;
    RECT rect;
    POINT point;

    if (this->surfacesBackup[surface] == NULL)
    {
        return;
    }
    if (g_App.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) != D3D_OK)
    {
        return;
    }
    if (this->surfaces[surface] == NULL)
    {
        if (g_App.d3dDevice->CreateRenderTarget(
                this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                g_App.presentParams.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                &this->surfaces[surface]) != D3D_OK &&
            g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width,
                                                         this->surfaceInfos[surface].Height,
                                                         g_App.presentParams.BackBufferFormat,
                                                         &this->surfaces[surface]) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
        if (D3DXLoadSurfaceFromSurface(this->surfaces[surface], NULL, NULL, this->surfacesBackup[surface], NULL, NULL,
                                       D3DX_FILTER_NONE, 0) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
    }
    rect.left = left;
    rect.top = top;
    rect.right = this->surfaceInfos[surface].Width;
    rect.bottom = this->surfaceInfos[surface].Height;
    point.x = x;
    point.y = y;
    g_App.d3dDevice->CopyRects(this->surfaces[surface], &rect, 1, backBuffer, &point);
    backBuffer->Release();
}

void SpriteEngine::CopySurfaceRectToBackBuffer(int surface, int x, int y, int left, int top, int width, int height)
{
    IDirect3DSurface8 *backBuffer;
    RECT rect;
    POINT point;

    if (this->surfacesBackup[surface] == NULL)
    {
        return;
    }
    if (g_App.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) != D3D_OK)
    {
        return;
    }
    if (this->surfaces[surface] == NULL)
    {
        if (g_App.d3dDevice->CreateRenderTarget(
                this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                g_App.presentParams.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                &this->surfaces[surface]) != D3D_OK &&
            g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width,
                                                         this->surfaceInfos[surface].Height,
                                                         g_App.presentParams.BackBufferFormat,
                                                         &this->surfaces[surface]) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
        if (D3DXLoadSurfaceFromSurface(this->surfaces[surface], NULL, NULL, this->surfacesBackup[surface], NULL, NULL,
                                       D3DX_FILTER_NONE, 0) != D3D_OK)
        {
            backBuffer->Release();
            return;
        }
    }
    rect.left = left;
    rect.top = top;
    rect.right = left + width;
    rect.bottom = top + height;
    point.x = x;
    point.y = y;
    g_App.d3dDevice->CopyRects(this->surfaces[surface], &rect, 1, backBuffer, &point);
    backBuffer->Release();
}

void SpriteEngine::CopyBackBufferToTexture(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight,
                                           int dstLeft, int dstTop, int dstWidth, int dstHeight)
{
    IDirect3DSurface8 *backBuffer;
    IDirect3DSurface8 *dest;
    RECT rect;
    RECT destinationRect;

    if (this->anms[texture].textures[0].texture == NULL)
    {
        return;
    }
    this->FlushQuadBatch();
    if (g_App.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) != D3D_OK)
    {
        return;
    }
    if (this->anms[texture].textures[0].texture->GetSurfaceLevel(0, &dest) != D3D_OK)
    {
        backBuffer->Release();
        return;
    }
    rect.left = srcLeft;
    rect.top = srcTop;
    rect.right = srcLeft + srcWidth;
    rect.bottom = srcTop + srcHeight;
    destinationRect.left = dstLeft;
    destinationRect.top = dstTop;
    destinationRect.right = dstLeft + dstWidth;
    destinationRect.bottom = dstTop + dstHeight;
    if (D3DXLoadSurfaceFromSurface(dest, NULL, &destinationRect, backBuffer, NULL, &rect, D3DX_DEFAULT, 0) != D3D_OK)
    {
        dest->Release();
        backBuffer->Release();
        return;
    }
    dest->Release();
    backBuffer->Release();
}

void SpriteEngine::CopyBackBufferToSurface(int surface, int srcLeft, int srcTop, int srcWidth, int srcHeight,
                                           int dstLeft, int dstTop, int dstWidth, int dstHeight)
{
    IDirect3DSurface8 *backBuffer;
    RECT rect;
    RECT destinationRect;

    this->FlushQuadBatch();
    if (this->surfaces[surface] != NULL)
    {
        this->ReleaseSurface(surface);
    }
    rect.left = srcLeft;
    rect.top = srcTop;
    rect.right = srcLeft + srcWidth;
    rect.bottom = srcTop + srcHeight;
    destinationRect.left = dstLeft;
    destinationRect.top = dstTop;
    destinationRect.right = dstLeft + dstWidth;
    destinationRect.bottom = dstTop + dstHeight;
    if (g_App.d3dDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &backBuffer) != D3D_OK)
    {
        return;
    }
    this->surfaceInfos[surface].Width = dstWidth;
    this->surfaceInfos[surface].Height = dstHeight;
    if (g_App.d3dDevice->CreateRenderTarget(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                                            &this->surfaces[surface]) != D3D_OK &&
        g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat,
                                            &this->surfaces[surface]) != D3D_OK)
    {
        goto done;
    }
    if (g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat,
                                            &this->surfacesBackup[surface]) != D3D_OK)
    {
        goto done;
    }
    if (D3DXLoadSurfaceFromSurface(this->surfaces[surface], NULL, &destinationRect, backBuffer, NULL, &rect,
                                   D3DX_DEFAULT, 0) != D3D_OK)
    {
        goto done;
    }
    D3DXLoadSurfaceFromSurface(this->surfacesBackup[surface], NULL, NULL, this->surfaces[surface], NULL, NULL,
                               D3DX_DEFAULT, 0);

done:
    if (backBuffer != NULL)
    {
        backBuffer->Release();
        backBuffer = NULL;
    }
}
