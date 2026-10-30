#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "gdisurface.h"
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
