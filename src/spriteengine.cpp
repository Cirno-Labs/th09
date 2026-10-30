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
