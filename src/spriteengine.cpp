#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "gdisurface.h"
#include "unsorted.h"
#include "sprite.h"

// Scales color channel `c` by `factor`, where 0x80 is 1, clamping to
// 0xff, as stage.cpp's: the linker folds the copies (0x401090).
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

int AnmLoaded::SetSprite(Sprite *vm, int sprite)
{
    if (this->file == NULL || this->loadStep != 0)
    {
        return -1;
    }
    vm->anm = this;
    vm->spriteIndex = sprite;
    vm->spriteData = &this->sprites[sprite];
    vm->spriteWidth = vm->spriteData->width;
    vm->spriteHeight = vm->spriteData->height;
    D3DXMatrixIdentity(&vm->matrix);
    D3DXMatrixIdentity(&vm->sizeMatrix);
    vm->matrix.m[0][0] = vm->spriteWidth / 256.0f;
    vm->matrix.m[1][1] = vm->spriteHeight / 256.0f;
    vm->sizeMatrix.m[0][0] = vm->spriteData->scaleX / vm->spriteData->textureWidth * vm->spriteWidth;
    vm->sizeMatrix.m[1][1] = vm->spriteData->scaleY / vm->spriteData->textureHeight * vm->spriteHeight;
    vm->drawMatrix = vm->matrix;
    return 0;
}

int Sprite::ResolveIntArgument(int value)
{
    switch (value)
    {
    case 10000:
        return this->scriptVar0;
    case 10001:
        return this->scriptVar1;
    case 10002:
        return this->scriptVar2;
    case 10003:
        return this->scriptVar3;
    case 10004:
        return this->scriptVarF0;
    case 10005:
        return this->scriptVarF1;
    case 10006:
        return this->scriptVarF2;
    case 10007:
        return this->scriptVarF3;
    case 10008:
        return this->scriptVar8;
    case 10009:
        return this->scriptVar9;
    }
    return value;
}

float *Sprite::ResolveFloatArgumentPtr(float *value, WORD mask, int bit)
{
    if ((mask & 1 << bit) == 0)
    {
        return value;
    }
    switch ((int)*value)
    {
    case 10004:
        return &this->scriptVarF0;
    case 10005:
        return &this->scriptVarF1;
    case 10006:
        return &this->scriptVarF2;
    case 10007:
        return &this->scriptVarF3;
    }
    return value;
}

int *Sprite::ResolveIntArgumentPtr(int *value, WORD mask, int bit)
{
    if ((mask & 1 << bit) == 0)
    {
        return value;
    }
    switch (*value)
    {
    case 10000:
        return &this->scriptVar0;
    case 10001:
        return &this->scriptVar1;
    case 10002:
        return &this->scriptVar2;
    case 10003:
        return &this->scriptVar3;
    case 10008:
        return &this->scriptVar8;
    case 10009:
        return &this->scriptVar9;
    }
    return value;
}

void SpriteEngine::DrawSprite(Sprite *vm)
{
    switch (vm->drawMode)
    {
    case 0:
        this->DrawUnrotated(vm);
        break;
    case 1:
        this->DrawNoRound(vm);
        break;
    case 2:
        this->Draw(vm);
        break;
    case 3:
        this->DrawRotatedNoRound(vm);
        break;
    case 4:
        this->DrawProjected(vm);
        break;
    case 5:
        this->DrawProjected3D(vm);
        break;
    }
}

void SpriteEngine::SendInterruptToLoadedSprites(Sprite *vms, int count, short interrupt)
{
    while (count != 0)
    {
        if (g_SpriteEngine->HasTexture(vms))
        {
            vms->SetPendingInterrupt(interrupt);
        }
        vms++;
        count--;
    }
}

float Sprite::ResolveFloatArgument(float value)
{
    switch ((int)value)
    {
    case 10000:
        return this->scriptVar0;
    case 10001:
        return this->scriptVar1;
    case 10002:
        return this->scriptVar2;
    case 10003:
        return this->scriptVar3;
    case 10004:
        return this->scriptVarF0;
    case 10005:
        return this->scriptVarF1;
    case 10006:
        return this->scriptVarF2;
    case 10007:
        return this->scriptVarF3;
    case 10008:
        return this->scriptVar8;
    case 10009:
        return this->scriptVar9;
    case 10010:
        return g_Rng.GetRandomF32ZeroToOne();
    case 10011:
        return g_Rng.GetRandomF32MinusOneToOne();
    case 10012:
        return g_Rng.GetRandomF32SignedInRange(3.1415927f);
    }
    return value;
}

void SpriteEngine::StepSprites(Sprite *vms, int count)
{
    while (count != 0)
    {
        if (vms->scriptIndex >= 0)
        {
            g_SpriteEngine->Step(vms);
        }
        vms++;
        count--;
    }
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

void AnmLoaded::StartSpriteSeries(Sprite *vms, int script, int count)
{
    while (count != 0)
    {
        this->StartScriptAtOrigin(vms, script);
        vms->baseSpriteIndex = vms->spriteIndex;
        script++;
        vms++;
        count--;
    }
}

void SpriteEngine::BeginQuadBatch()
{
    this->batchQuadCount = 0;
    this->batchCursor = this->batchVertices;
    this->batchStart = this->batchCursor;
}

void SpriteEngine::FlushQuadBatch()
{
    if (this->batchQuadCount == 0)
    {
        return;
    }
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, this->batchQuadCount * 2, this->batchStart,
                                     sizeof(VertexTex1DiffuseXyzrwh));
    this->batchStart = this->batchCursor;
    this->batchQuadCount = 0;
    this->batchDrawCount++;
}

int SpriteEngine::AddQuadToBatch(VertexTex1DiffuseXyzrwh *quad)
{
    this->batchCursor[0] = quad[0];
    this->batchCursor[1] = quad[1];
    this->batchCursor[2] = quad[2];
    this->batchCursor[3] = quad[1];
    this->batchCursor[4] = quad[2];
    this->batchCursor[5] = quad[3];
    this->batchCursor += 6;
    this->batchQuadCount++;
    return 0;
}

void SpriteEngine::RotatePoint(float *out, float x, float y, float sine, float cosine, float cx, float cy)
{
    out[0] = x * cosine - y * sine + cx;
    out[1] = x * sine + y * cosine + cy;
}

int SpriteEngine::ProjectSprite(Sprite *vm)
{
    float c;
    float angle;
    float s;
    float x;
    float y;
    float h;
    float w;

    angle = vm->rotation.z;
    __asm
    {
        fld angle
        fsincos
        fstp c
        fstp s
    }
    D3DXMATRIX world;
    D3DXVECTOR3 origin;
    D3DXVECTOR3 edge;
    D3DXVECTOR3 d;
    D3DXVECTOR3 o(0.0f, 0.0f, 0.0f);

    // origin is o, the sprite's position, projected to the screen.
    D3DXMatrixIdentity(&world);
    memcpy(&world._41, vm->pos, sizeof(D3DXVECTOR3));
    D3DXVec3Project(&origin, &o, &g_App.camera->viewport, &g_App.camera->projectionMatrix,
                    &g_App.camera->viewMatrix, &world);
    if (origin.z < 0.0f || origin.z > 1.0f)
    {
        return -1;
    }
    D3DXVec3Project(&edge, &g_App.camera->cameraRight, &g_App.camera->viewport, &g_App.camera->projectionMatrix,
                    &g_App.camera->viewMatrix, &world);
    d = edge - origin;
    x = D3DXVec3Length(&d) * 0.5f;
    w = vm->spriteWidth * vm->scaleY * x;
    h = vm->spriteHeight * vm->scaleX * x;
    x = origin.x;
    y = origin.y;
    this->RotatePoint((float *)&g_DiffuseQuad[0].pos, -w, -h, s, c, x, y);
    this->RotatePoint((float *)&g_DiffuseQuad[1].pos, w, -h, s, c, x, y);
    this->RotatePoint((float *)&g_DiffuseQuad[2].pos, -w, h, s, c, x, y);
    this->RotatePoint((float *)&g_DiffuseQuad[3].pos, w, h, s, c, x, y);
    g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = origin.z;
    if ((vm->flags >> 11 & 3) & 1)
    {
        g_DiffuseQuad[0].pos.x += w;
        g_DiffuseQuad[1].pos.x += w;
        g_DiffuseQuad[2].pos.x += w;
        g_DiffuseQuad[3].pos.x += w;
    }
    if ((vm->flags >> 11 & 3) & 2)
    {
        g_DiffuseQuad[0].pos.y += h;
        g_DiffuseQuad[1].pos.y += h;
        g_DiffuseQuad[2].pos.y += h;
        g_DiffuseQuad[3].pos.y += h;
    }
    return 0;
}

void SpriteEngine::ProjectSprite3D(Sprite *vm)
{
    D3DXMATRIX worldMatrix;
    D3DXMATRIX rotationMatrix;
    DWORD anchor;

    if ((vm->flags >> 16 & 1) == 0 && ((vm->flags >> 3 & 1) != 0 || (vm->flags >> 2 & 1) != 0))
    {
        vm->drawMatrix = vm->matrix;
        vm->drawMatrix._11 *= vm->scaleY;
        vm->drawMatrix._22 *= vm->scaleX;
        vm->flags &= ~8;
        if (vm->rotation.x != 0.0)
        {
            D3DXMatrixRotationX(&rotationMatrix, vm->rotation.x);
            D3DXMatrixMultiply(&vm->drawMatrix, &vm->drawMatrix, &rotationMatrix);
        }
        if (vm->rotation.y != 0.0)
        {
            D3DXMatrixRotationY(&rotationMatrix, vm->rotation.y);
            D3DXMatrixMultiply(&vm->drawMatrix, &vm->drawMatrix, &rotationMatrix);
        }
        if (vm->rotation.z != 0.0)
        {
            D3DXMatrixRotationZ(&rotationMatrix, vm->rotation.z);
            D3DXMatrixMultiply(&vm->drawMatrix, &vm->drawMatrix, &rotationMatrix);
        }
        vm->flags &= ~4;
    }
    worldMatrix = vm->drawMatrix;
    anchor = vm->flags >> 11;
    if ((anchor & 1) == 0)
    {
        worldMatrix._41 = vm->pos.x;
    }
    else
    {
        worldMatrix._41 = Fabs(vm->spriteWidth * vm->scaleY / 2.0f) + vm->pos.x;
    }
    if ((anchor & 2) == 0)
    {
        worldMatrix._42 = vm->pos.y;
    }
    else
    {
        worldMatrix._42 = Fabs(vm->spriteHeight * vm->scaleX / 2.0f) + vm->pos.y;
    }
    worldMatrix._43 = vm->pos.z;
    D3DXVec3Project(&g_DiffuseQuad[0].pos, &this->quad[0].pos, &g_App.camera->viewport,
                    &g_App.camera->projectionMatrix, &g_App.camera->viewMatrix, &worldMatrix);
    D3DXVec3Project(&g_DiffuseQuad[1].pos, &this->quad[1].pos, &g_App.camera->viewport,
                    &g_App.camera->projectionMatrix, &g_App.camera->viewMatrix, &worldMatrix);
    D3DXVec3Project(&g_DiffuseQuad[2].pos, &this->quad[2].pos, &g_App.camera->viewport,
                    &g_App.camera->projectionMatrix, &g_App.camera->viewMatrix, &worldMatrix);
    D3DXVec3Project(&g_DiffuseQuad[3].pos, &this->quad[3].pos, &g_App.camera->viewport,
                    &g_App.camera->projectionMatrix, &g_App.camera->viewMatrix, &worldMatrix);
    this->lastWorldMatrix = worldMatrix;
}

void SpriteEngine::SetRenderStateForSprite(Sprite *vm)
{
    D3DCOLOR color;

    if (this->blendMode != (vm->flags >> 4 & 3))
    {
        this->FlushQuadBatch();
        this->blendMode = vm->flags >> 4 & 3;
        switch (this->blendMode)
        {
        case 0:
            g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            break;
        case 1:
            g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
            break;
        }
    }
    color = (vm->flags >> 17 & 1) ? vm->color2 : vm->color;
    if (this->modulateColorEnabled != 0)
    {
        MODULATE_CHANNEL(color, this->modulateColor, 2);
        MODULATE_CHANNEL(color, this->modulateColor, 1);
        MODULATE_CHANNEL(color, this->modulateColor, 0);
        MODULATE_CHANNEL(color, this->modulateColor, 3);
    }
    if (this->currentColor != color)
    {
        this->FlushQuadBatch();
        this->currentColor = color;
        g_App.d3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, this->currentColor);
    }
    if (this->zWriteDisabled != (vm->flags >> 13 & 1))
    {
        this->FlushQuadBatch();
        this->zWriteDisabled = vm->flags >> 13 & 1;
        if (this->zWriteDisabled == 0)
        {
            g_App.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        }
        else
        {
            g_App.d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        }
    }
    if (this->lastCameraSetupFlag != (vm->flags >> 15 & 1))
    {
        this->FlushQuadBatch();
        this->lastCameraSetupFlag = vm->flags >> 15 & 1;
        if (this->lastCameraSetupFlag == 0)
        {
            g_App.ResetView();
        }
        else
        {
            g_App.ResetView3D();
        }
    }
    this->renderStateSetCount++;
}

int SpriteEngine::FillSpriteStripVertices(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count)
{
    float width;
    float step;
    float u;
    float u0;
    VertexTex1DiffuseXyzrwh *cur;
    int i;
    float v;

    if (count < 3)
    {
        return -1;
    }
    u0 = vm->spriteData->uvEndX + vm->uvScrollX;
    width = vm->spriteData->uvEndX - vm->spriteData->uvStartX;
    v = vm->spriteData->uvStartY + vm->uvScrollY;
    cur = vertices;
    step = width / ((count + 1) / 2 - 1);
    for (i = 0, u = u0; i < count; i += 2, cur += 2, u -= step)
    {
        cur->uv.x = u;
        cur->uv.y = v;
        cur->diffuse = vm->color;
        cur->w = 1.0f;
    }
    v = vm->spriteData->uvEndY + vm->uvScrollY;
    cur = vertices + 1;
    for (i = 1, u = u0; i < count; i += 2, cur += 2, u -= step)
    {
        cur->uv.x = u;
        cur->uv.y = v;
        cur->diffuse = vm->color;
        cur->w = 1.0f;
    }
    return 0;
}

int SpriteEngine::FillVerticalStripVertices(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count)
{
    float width;
    float step;
    float u;
    float u0;
    VertexTex1DiffuseXyzrwh *cur;
    int i;
    float v;

    // FillSpriteStripVertices turned a quarter: u0 and u run down the
    // sprite's v coordinates, v is the fixed u coordinate.
    if (count < 3)
    {
        return -1;
    }
    u0 = vm->spriteData->uvEndY + vm->uvScrollY;
    width = vm->spriteData->uvEndY - vm->spriteData->uvStartY;
    v = vm->spriteData->uvStartX + vm->uvScrollX;
    cur = vertices;
    step = width / ((count + 1) / 2 - 1);
    for (i = 0, u = u0; i < count; i += 2, cur += 2, u -= step)
    {
        cur->uv.y = u;
        cur->uv.x = v;
        cur->diffuse = vm->color;
        cur->w = 1.0f;
    }
    v = vm->spriteData->uvEndX + vm->uvScrollX;
    cur = vertices + 1;
    for (i = 1, u = u0; i < count; i += 2, cur += 2, u -= step)
    {
        cur->uv.y = u;
        cur->uv.x = v;
        cur->diffuse = vm->color;
        cur->w = 1.0f;
    }
    return 0;
}

void SpriteEngine::SetBlendModeForSprite(Sprite *vm)
{
    if (this->blendMode != (vm->flags >> 4 & 3))
    {
        this->FlushQuadBatch();
        this->blendMode = vm->flags >> 4 & 3;
        switch (this->blendMode)
        {
        case 0:
            g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            break;
        case 1:
            g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
            break;
        }
    }
    if (this->zWriteDisabled != (vm->flags >> 13 & 1))
    {
        this->zWriteDisabled = vm->flags >> 13 & 1;
        if (this->zWriteDisabled == 0)
        {
            g_App.SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        }
        else
        {
            g_App.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        }
    }
    this->renderStateSetCount++;
}

int SpriteEngine::DrawQuad(Sprite *vm, int flags)
{
    float bottom;
    float minY;
    float minX;
    float maxX;
    D3DCOLOR diffuse;

    g_DiffuseQuad[0].pos.x += this->shakeX;
    g_DiffuseQuad[0].pos.y += this->shakeY;
    g_DiffuseQuad[1].pos.x += this->shakeX;
    g_DiffuseQuad[1].pos.y += this->shakeY;
    g_DiffuseQuad[2].pos.x += this->shakeX;
    g_DiffuseQuad[2].pos.y += this->shakeY;
    g_DiffuseQuad[3].pos.x += this->shakeX;
    g_DiffuseQuad[3].pos.y += this->shakeY;
    if (flags & 1)
    {
        __asm
        {
            fld dword ptr [g_DiffuseQuad + 0x00]
            frndint
            fsub g_Half
            fld dword ptr [g_DiffuseQuad + 0x1c]
            frndint
            fsub g_Half
            fld dword ptr [g_DiffuseQuad + 0x04]
            frndint
            fsub g_Half
            fld dword ptr [g_DiffuseQuad + 0x3c]
            frndint
            fsub g_Half
            fst dword ptr [g_DiffuseQuad + 0x3c]
            fstp dword ptr [g_DiffuseQuad + 0x58]
            fst dword ptr [g_DiffuseQuad + 0x04]
            fstp dword ptr [g_DiffuseQuad + 0x20]
            fst dword ptr [g_DiffuseQuad + 0x1c]
            fstp dword ptr [g_DiffuseQuad + 0x54]
            fst dword ptr [g_DiffuseQuad + 0x00]
            fstp dword ptr [g_DiffuseQuad + 0x38]
        }
    }
    g_DiffuseQuad[0].uv.x = g_DiffuseQuad[2].uv.x = vm->spriteData->uvStartX + vm->uvScrollX;
    g_DiffuseQuad[1].uv.x = g_DiffuseQuad[3].uv.x = vm->spriteData->uvEndX + vm->uvScrollX;
    g_DiffuseQuad[0].uv.y = g_DiffuseQuad[1].uv.y = vm->spriteData->uvStartY + vm->uvScrollY;
    g_DiffuseQuad[2].uv.y = g_DiffuseQuad[3].uv.y = vm->spriteData->uvEndY + vm->uvScrollY;
    maxX = g_DiffuseQuad[0].pos.x > g_DiffuseQuad[1].pos.x ? g_DiffuseQuad[0].pos.x : g_DiffuseQuad[1].pos.x;
    maxX = g_DiffuseQuad[2].pos.x > maxX ? g_DiffuseQuad[2].pos.x : maxX;
    maxX = g_DiffuseQuad[3].pos.x > maxX ? g_DiffuseQuad[3].pos.x : maxX;
    bottom = g_DiffuseQuad[0].pos.y > g_DiffuseQuad[1].pos.y ? g_DiffuseQuad[0].pos.y : g_DiffuseQuad[1].pos.y;
    bottom = g_DiffuseQuad[2].pos.y > bottom ? g_DiffuseQuad[2].pos.y : bottom;
    bottom = g_DiffuseQuad[3].pos.y > bottom ? g_DiffuseQuad[3].pos.y : bottom;
    minX = g_DiffuseQuad[0].pos.x < g_DiffuseQuad[1].pos.x ? g_DiffuseQuad[0].pos.x : g_DiffuseQuad[1].pos.x;
    minX = g_DiffuseQuad[2].pos.x < minX ? g_DiffuseQuad[2].pos.x : minX;
    minX = g_DiffuseQuad[3].pos.x < minX ? g_DiffuseQuad[3].pos.x : minX;
    minY = g_DiffuseQuad[0].pos.y < g_DiffuseQuad[1].pos.y ? g_DiffuseQuad[0].pos.y : g_DiffuseQuad[1].pos.y;
    minY = g_DiffuseQuad[2].pos.y < minY ? g_DiffuseQuad[2].pos.y : minY;
    minY = g_DiffuseQuad[3].pos.y < minY ? g_DiffuseQuad[3].pos.y : minY;
    if (maxX < g_App.camera->viewport.X || g_App.camera->viewport.Y > bottom ||
        g_App.camera->viewport.X + g_App.camera->viewport.Width < minX ||
        g_App.camera->viewport.Y + g_App.camera->viewport.Height < minY)
    {
        return 0;
    }
    if (this->currentTexture != vm->spriteData->texture)
    {
        this->currentTexture = vm->spriteData->texture;
        this->FlushQuadBatch();
        g_App.d3dDevice->SetTexture(0, this->currentTexture);
    }
    if (this->vertexFormat != 1)
    {
        this->FlushQuadBatch();
        this->vertexFormat = 1;
    }
    if ((flags & 2) == 0)
    {
        diffuse = (vm->flags >> 17 & 1) ? vm->color2 : vm->color;
        if (this->modulateColorEnabled != 0)
        {
            MODULATE_CHANNEL(diffuse, this->modulateColor, 2);
            MODULATE_CHANNEL(diffuse, this->modulateColor, 1);
            MODULATE_CHANNEL(diffuse, this->modulateColor, 0);
            MODULATE_CHANNEL(diffuse, this->modulateColor, 3);
        }
        g_DiffuseQuad[0].diffuse = diffuse;
        g_DiffuseQuad[1].diffuse = diffuse;
        g_DiffuseQuad[2].diffuse = diffuse;
        g_DiffuseQuad[3].diffuse = diffuse;
    }
    this->SetBlendModeForSprite(vm);
    this->AddQuadToBatch(g_DiffuseQuad);
    return 0;
}

int SpriteEngine::DrawQuadMirrored(Sprite *vm, int flags)
{
    float bottom;
    float minY;
    float minX;
    float maxX;
    D3DCOLOR diffuse;

    g_DiffuseQuad[0].pos.x += this->shakeX;
    g_DiffuseQuad[0].pos.y += this->shakeY;
    g_DiffuseQuad[1].pos.x += this->shakeX;
    g_DiffuseQuad[1].pos.y += this->shakeY;
    g_DiffuseQuad[2].pos.x += this->shakeX;
    g_DiffuseQuad[2].pos.y += this->shakeY;
    g_DiffuseQuad[3].pos.x += this->shakeX;
    g_DiffuseQuad[3].pos.y += this->shakeY;
    if (flags & 1)
    {
        __asm
        {
            fld dword ptr [g_DiffuseQuad + 0x00]
            frndint
            fsub g_Half
            fld dword ptr [g_DiffuseQuad + 0x1c]
            frndint
            fsub g_Half
            fld dword ptr [g_DiffuseQuad + 0x04]
            frndint
            fsub g_Half
            fld dword ptr [g_DiffuseQuad + 0x3c]
            frndint
            fsub g_Half
            fst dword ptr [g_DiffuseQuad + 0x3c]
            fstp dword ptr [g_DiffuseQuad + 0x58]
            fst dword ptr [g_DiffuseQuad + 0x04]
            fstp dword ptr [g_DiffuseQuad + 0x20]
            fst dword ptr [g_DiffuseQuad + 0x1c]
            fstp dword ptr [g_DiffuseQuad + 0x54]
            fst dword ptr [g_DiffuseQuad + 0x00]
            fstp dword ptr [g_DiffuseQuad + 0x38]
        }
    }
    g_DiffuseQuad[0].uv.x = g_DiffuseQuad[2].uv.x = vm->spriteData->uvEndX + vm->uvScrollX;
    g_DiffuseQuad[1].uv.x = g_DiffuseQuad[3].uv.x = vm->spriteData->uvStartX + vm->uvScrollX;
    g_DiffuseQuad[0].uv.y = g_DiffuseQuad[1].uv.y = vm->spriteData->uvStartY + vm->uvScrollY;
    g_DiffuseQuad[2].uv.y = g_DiffuseQuad[3].uv.y = vm->spriteData->uvEndY + vm->uvScrollY;
    maxX = g_DiffuseQuad[0].pos.x > g_DiffuseQuad[1].pos.x ? g_DiffuseQuad[0].pos.x : g_DiffuseQuad[1].pos.x;
    maxX = g_DiffuseQuad[2].pos.x > maxX ? g_DiffuseQuad[2].pos.x : maxX;
    maxX = g_DiffuseQuad[3].pos.x > maxX ? g_DiffuseQuad[3].pos.x : maxX;
    bottom = g_DiffuseQuad[0].pos.y > g_DiffuseQuad[1].pos.y ? g_DiffuseQuad[0].pos.y : g_DiffuseQuad[1].pos.y;
    bottom = g_DiffuseQuad[2].pos.y > bottom ? g_DiffuseQuad[2].pos.y : bottom;
    bottom = g_DiffuseQuad[3].pos.y > bottom ? g_DiffuseQuad[3].pos.y : bottom;
    minX = g_DiffuseQuad[0].pos.x < g_DiffuseQuad[1].pos.x ? g_DiffuseQuad[0].pos.x : g_DiffuseQuad[1].pos.x;
    minX = g_DiffuseQuad[2].pos.x < minX ? g_DiffuseQuad[2].pos.x : minX;
    minX = g_DiffuseQuad[3].pos.x < minX ? g_DiffuseQuad[3].pos.x : minX;
    minY = g_DiffuseQuad[0].pos.y < g_DiffuseQuad[1].pos.y ? g_DiffuseQuad[0].pos.y : g_DiffuseQuad[1].pos.y;
    minY = g_DiffuseQuad[2].pos.y < minY ? g_DiffuseQuad[2].pos.y : minY;
    minY = g_DiffuseQuad[3].pos.y < minY ? g_DiffuseQuad[3].pos.y : minY;
    if (maxX < g_App.camera->viewport.X || g_App.camera->viewport.Y > bottom ||
        g_App.camera->viewport.X + g_App.camera->viewport.Width < minX ||
        g_App.camera->viewport.Y + g_App.camera->viewport.Height < minY)
    {
        return 0;
    }
    if (this->currentTexture != vm->spriteData->texture)
    {
        this->currentTexture = vm->spriteData->texture;
        this->FlushQuadBatch();
        g_App.d3dDevice->SetTexture(0, this->currentTexture);
    }
    if (this->vertexFormat != 1)
    {
        this->FlushQuadBatch();
        this->vertexFormat = 1;
    }
    if ((flags & 2) == 0)
    {
        diffuse = (vm->flags >> 17 & 1) ? vm->color2 : vm->color;
        if (this->modulateColorEnabled != 0)
        {
            MODULATE_CHANNEL(diffuse, this->modulateColor, 2);
            MODULATE_CHANNEL(diffuse, this->modulateColor, 1);
            MODULATE_CHANNEL(diffuse, this->modulateColor, 0);
            MODULATE_CHANNEL(diffuse, this->modulateColor, 3);
        }
        g_DiffuseQuad[0].diffuse = diffuse;
        g_DiffuseQuad[1].diffuse = diffuse;
        g_DiffuseQuad[2].diffuse = diffuse;
        g_DiffuseQuad[3].diffuse = diffuse;
    }
    this->SetBlendModeForSprite(vm);
    this->AddQuadToBatch(g_DiffuseQuad);
    return 0;
}

int SpriteEngine::DrawUnrotated(Sprite *vm)
{
    float w;
    float h;

    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    w = vm->spriteWidth * vm->scaleY / 2.0f;
    h = vm->spriteHeight * vm->scaleX / 2.0f;
    if (((vm->flags >> 11 & 3) & 1) == 0)
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x - w;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + vm->pos.x;
    }
    else
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + w + vm->pos.x;
    }
    if (((vm->flags >> 11 & 3) & 2) == 0)
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y - h;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + vm->pos.y;
    }
    else
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + h + vm->pos.y;
    }
    g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = vm->pos.z;
    return this->DrawQuad(vm, 1);
}

int SpriteEngine::DrawUnrotatedMirrored(Sprite *vm)
{
    float w;
    float h;

    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    w = vm->spriteWidth * vm->scaleY / 2.0f;
    h = vm->spriteHeight * vm->scaleX / 2.0f;
    if (((vm->flags >> 11 & 3) & 1) == 0)
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x - w;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + vm->pos.x;
    }
    else
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + w + vm->pos.x;
    }
    if (((vm->flags >> 11 & 3) & 2) == 0)
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y - h;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + vm->pos.y;
    }
    else
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + h + vm->pos.y;
    }
    g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = vm->pos.z;
    return this->DrawQuadMirrored(vm, 1);
}

int SpriteEngine::Draw(Sprite *vm)
{
    float x0;
    float cy;
    float halfH;
    float halfWidth;
    float cosine;
    float angle;
    float s;

    if (vm->rotation.z == 0.0f)
    {
        return this->DrawUnrotated(vm);
    }
    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    angle = vm->rotation.z;
    __asm
    {
        fld angle
        fsincos
        fstp cosine
        fstp s
    }
    x0 = vm->pos.x;
    cy = vm->pos.y;
    halfWidth = vm->spriteWidth * vm->scaleY / 2.0f;
    halfH = vm->spriteHeight * vm->scaleX / 2.0f;
    this->RotatePoint((float *)&g_DiffuseQuad[0].pos, -halfWidth, -halfH, s, cosine, x0, cy);
    this->RotatePoint((float *)&g_DiffuseQuad[1].pos, halfWidth, -halfH, s, cosine, x0, cy);
    this->RotatePoint((float *)&g_DiffuseQuad[2].pos, -halfWidth, halfH, s, cosine, x0, cy);
    this->RotatePoint((float *)&g_DiffuseQuad[3].pos, halfWidth, halfH, s, cosine, x0, cy);
    g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = vm->pos.z;
    if ((vm->flags >> 11 & 3) & 1)
    {
        g_DiffuseQuad[0].pos.x += halfWidth;
        g_DiffuseQuad[1].pos.x += halfWidth;
        g_DiffuseQuad[2].pos.x += halfWidth;
        g_DiffuseQuad[3].pos.x += halfWidth;
    }
    if ((vm->flags >> 11 & 3) & 2)
    {
        g_DiffuseQuad[0].pos.y += halfH;
        g_DiffuseQuad[1].pos.y += halfH;
        g_DiffuseQuad[2].pos.y += halfH;
        g_DiffuseQuad[3].pos.y += halfH;
    }
    return this->DrawQuad(vm, 0);
}

int SpriteEngine::DrawRotatedNoRound(Sprite *vm)
{
    float x0;
    float cy;
    float halfH;
    float halfWidth;
    float cosine;
    float angle;
    float s;

    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    angle = vm->rotation.z;
    if (angle != 0.0f)
    {
        __asm
        {
            fld angle
            fsincos
            fstp cosine
            fstp s
        }
        x0 = vm->pos.x;
        cy = vm->pos.y;
        halfWidth = vm->spriteWidth * vm->scaleY / 2.0f;
        halfH = vm->spriteHeight * vm->scaleX / 2.0f;
        this->RotatePoint((float *)&g_DiffuseQuad[0].pos, -halfWidth, -halfH, s, cosine, x0, cy);
        this->RotatePoint((float *)&g_DiffuseQuad[1].pos, halfWidth, -halfH, s, cosine, x0, cy);
        this->RotatePoint((float *)&g_DiffuseQuad[2].pos, -halfWidth, halfH, s, cosine, x0, cy);
        this->RotatePoint((float *)&g_DiffuseQuad[3].pos, halfWidth, halfH, s, cosine, x0, cy);
        g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = vm->pos.z;
        if ((vm->flags >> 11 & 3) & 1)
        {
            g_DiffuseQuad[0].pos.x += halfWidth;
            g_DiffuseQuad[1].pos.x += halfWidth;
            g_DiffuseQuad[2].pos.x += halfWidth;
            g_DiffuseQuad[3].pos.x += halfWidth;
        }
        if ((vm->flags >> 11 & 3) & 2)
        {
            g_DiffuseQuad[0].pos.y += halfH;
            g_DiffuseQuad[1].pos.y += halfH;
            g_DiffuseQuad[2].pos.y += halfH;
            g_DiffuseQuad[3].pos.y += halfH;
        }
    }
    else
    {
        float w;
        float h;

        // Unlike DrawUnrotated, leaves the quad's z as it was.
        w = vm->spriteWidth * vm->scaleY / 2.0f;
        h = vm->spriteHeight * vm->scaleX / 2.0f;
        if (((vm->flags >> 11 & 3) & 1) == 0)
        {
            g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x - w;
            g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + vm->pos.x;
        }
        else
        {
            g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x;
            g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + vm->pos.x + w;
        }
        if (((vm->flags >> 11 & 3) & 2) == 0)
        {
            g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y - h;
            g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + vm->pos.y;
        }
        else
        {
            g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y;
            g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + vm->pos.y + h;
        }
    }
    return this->DrawQuad(vm, 0);
}

int SpriteEngine::DrawNoRound(Sprite *vm)
{
    float w;
    float h;

    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    w = vm->spriteWidth * vm->scaleY / 2.0f;
    h = vm->spriteHeight * vm->scaleX / 2.0f;
    if (((vm->flags >> 11 & 3) & 1) == 0)
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x - w;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + vm->pos.x;
    }
    else
    {
        g_DiffuseQuad[0].pos.x = g_DiffuseQuad[2].pos.x = vm->pos.x;
        g_DiffuseQuad[1].pos.x = g_DiffuseQuad[3].pos.x = w + w + vm->pos.x;
    }
    if (((vm->flags >> 11 & 3) & 2) == 0)
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y - h;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + vm->pos.y;
    }
    else
    {
        g_DiffuseQuad[0].pos.y = g_DiffuseQuad[1].pos.y = vm->pos.y;
        g_DiffuseQuad[2].pos.y = g_DiffuseQuad[3].pos.y = h + h + vm->pos.y;
    }
    g_DiffuseQuad[0].pos.z = g_DiffuseQuad[1].pos.z = g_DiffuseQuad[2].pos.z = g_DiffuseQuad[3].pos.z = vm->pos.z;
    return this->DrawQuad(vm, 0);
}

int SpriteEngine::DrawProjected(Sprite *vm)
{
    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    if (this->ProjectSprite(vm) != 0)
    {
        return -1;
    }
    return this->DrawQuad(vm, 0);
}

int SpriteEngine::DrawProjected3D(Sprite *vm)
{
    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    this->ProjectSprite3D(vm);
    return this->DrawQuad(vm, 0);
}

int SpriteEngine::DrawVertexStrip(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count)
{
    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    if (this->batchQuadCount != 0)
    {
        this->FlushQuadBatch();
    }
    if (this->currentTexture != vm->spriteData->texture)
    {
        this->currentTexture = vm->spriteData->texture;
        g_App.d3dDevice->SetTexture(0, this->currentTexture);
    }
    if (this->vertexFormat != 3)
    {
        g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
        this->vertexFormat = 3;
    }
    this->SetBlendModeForSprite(vm);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, count - 2, vertices, sizeof(vertices[0]));
    return 0;
}

int SpriteEngine::DrawVertexQuad(Sprite *vm, VertexTex1DiffuseXyzrwh *quad)
{
    if (!vm->IsShown())
    {
        return -1;
    }
    if ((vm->flags >> 1 & 1) == 0)
    {
        return -1;
    }
    if (vm->alpha == 0)
    {
        return -1;
    }
    if (this->currentTexture != vm->spriteData->texture)
    {
        this->currentTexture = vm->spriteData->texture;
        this->FlushQuadBatch();
        g_App.d3dDevice->SetTexture(0, this->currentTexture);
    }
    if (this->vertexFormat != 1)
    {
        this->FlushQuadBatch();
        this->vertexFormat = 1;
    }
    this->SetBlendModeForSprite(vm);
    this->AddQuadToBatch(quad);
    return 0;
}

int SpriteEngine::DrawColoredStrip(Sprite *vm, VertexDiffuseXyzrwh *vertices, int count)
{
    if (this->batchQuadCount != 0)
    {
        this->FlushQuadBatch();
    }
    if (this->vertexFormat != 4)
    {
        g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        this->vertexFormat = 4;
    }
    this->SetBlendModeForSprite(vm);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    g_App.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, count - 2, vertices, sizeof(vertices[0]));
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    return 0;
}

int SpriteEngine::DrawColoredFan(Sprite *vm, VertexDiffuseXyzrwh *vertices, int count)
{
    if (this->batchQuadCount != 0)
    {
        this->FlushQuadBatch();
    }
    if (this->vertexFormat != 4)
    {
        g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        this->vertexFormat = 4;
    }
    this->SetBlendModeForSprite(vm);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    g_App.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, count - 2, vertices, sizeof(vertices[0]));
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    return 0;
}

int SpriteEngine::DrawVertexFan(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count)
{
    if (this->batchQuadCount != 0)
    {
        this->FlushQuadBatch();
    }
    if (this->vertexFormat != 3)
    {
        g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
        this->vertexFormat = 3;
    }
    this->SetBlendModeForSprite(vm);
    if (this->currentTexture != vm->spriteData->texture)
    {
        this->currentTexture = vm->spriteData->texture;
        g_App.d3dDevice->SetTexture(0, this->currentTexture);
    }
    g_App.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, count - 2, vertices, sizeof(vertices[0]));
    return 0;
}

// 0x43bba0
int SpriteEngine::CreateTexture(AnmTexture *tex, int width, int height, int format)
{
    D3DXCreateTexture(g_App.d3dDevice, width, height, 1, 0, g_TextureFormats[format], D3DPOOL_MANAGED, &tex->texture);
    return 0;
}

// 0x43bbe0
int SpriteEngine::PreloadAnmTexture(AnmLoaded *anm, int index, int *spriteIndex, int *scriptIndex, AnmFile *file)
{
    void *data;
    int size;
    char *name;
    AnmFile *anmHeader;
    int highest;

    highest = 0;
    if (file == NULL)
    {
        LogError(&g_Log, "アニメが読み込めません。データが失われてるか壊れています\r\n");
        return -1;
    }
    anmHeader = file;
    if (anmHeader->version != 3)
    {
        LogError(&g_Log, "アニメのバージョンが違います\r\n");
        return -1;
    }
    if (!anmHeader->hasImage)
    {
        name = (char *)anmHeader + anmHeader->nameOffset;
        if (*name != '@')
        {
            data = OpenPath(name, &size, 1);
            if (data == NULL)
            {
                LogError(&g_Log, "テクスチャ %s が読み込めません。データが失われてるか壊れています\r\n", name);
                return -1;
            }
            anm->textures[index].size = size;
            anm->textures[index].data = data;
        }
    }
    return highest + 1;
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

// `format`, or the 16-bit format to use instead of it when g_App forces
// 16-bit textures, as th08's SpriteEngine::AdjustTextureFormat (0x43c430).
static int AdjustTextureFormat(int format)
{
    if (g_App.GetForce16BitTextures())
    {
        if (g_TextureFormats[format] == D3DFMT_A8R8G8B8 || g_TextureFormats[format] == D3DFMT_UNKNOWN)
        {
            format = 5;
        }
        else if (g_TextureFormats[format] == D3DFMT_R8G8B8)
        {
            format = 3;
        }
    }
    return format;
}

// 0x43c470
int SpriteEngine::LoadTexture(AnmTexture *tex, int format, D3DCOLOR colorKey)
{
    format = AdjustTextureFormat(format);
    if (D3DXCreateTextureFromFileInMemoryEx(g_App.d3dDevice, tex->data, tex->size, 0, 0, 0, 0, g_TextureFormats[format],
                                            D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_DEFAULT, colorKey, NULL, NULL,
                                            &tex->texture) != D3D_OK)
    {
        return -1;
    }
    return 0;
}

// 0x43c4c0
int SpriteEngine::CreateTextureFromImage(AnmTexture *tex, AnmImage *image, int format)
{
    BYTE *dstRow;
    BYTE *srcRow;
    int y;
    D3DLOCKED_RECT rect;
    AnmImage *img;
    IDirect3DSurface8 *level;
    IDirect3DSurface8 *srcSurface;

    srcSurface = NULL;
    level = NULL;
    format = AdjustTextureFormat(format);
    img = image;
    g_App.d3dDevice->CreateImageSurface(img->width, img->height, g_TextureFormats[img->format], &srcSurface);
    srcSurface->LockRect(&rect, NULL, 0);
    for (y = 0; y < img->height; y++)
    {
        dstRow = (BYTE *)rect.pBits + y * rect.Pitch;
        srcRow = &image->data[y * img->width * g_TextureFormatSizes[img->format]];
        memcpy(dstRow, srcRow, img->width * g_TextureFormatSizes[img->format]);
    }
    srcSurface->UnlockRect();
    if (D3DXCreateTexture(g_App.d3dDevice, img->width, img->height, 1, 0, g_TextureFormats[format], D3DPOOL_MANAGED,
                          &tex->texture) != D3D_OK)
    {
        goto fail;
    }
    tex->texture->GetSurfaceLevel(0, &level);
    if (D3DXLoadSurfaceFromSurface(level, NULL, NULL, srcSurface, NULL, NULL, D3DX_FILTER_LINEAR, 0) != D3D_OK)
    {
        goto fail;
    }
    if (srcSurface != NULL)
    {
        srcSurface->Release();
        srcSurface = NULL;
    }
    if (level != NULL)
    {
        level->Release();
        level = NULL;
    }
    return 0;

fail:
    if (srcSurface != NULL)
    {
        srcSurface->Release();
        srcSurface = NULL;
    }
    if (level != NULL)
    {
        level->Release();
        level = NULL;
    }
    return -1;
}

// 0x43c610
AnmLoaded *SpriteEngine::PreloadAnmFile(int slot, char *path)
{
    AnmFile *header;
    int numScripts;
    int result;
    AnmFile *data;
    AnmLoaded *anm;
    int anmCount;
    int spriteCount;
    int i;

    if (slot >= 0x14)
    {
        LogError(&g_Log, "テクスチャ格納先が足りません\r\n");
        return NULL;
    }
    if (this->anms[slot].file != NULL)
    {
        this->anms[slot].releaseRequested = 1;
        while (this->anms[slot].releaseRequested != 0 && !g_App.closeSubThread)
        {
            Sleep(1);
        }
    }
    data = (AnmFile *)OpenPath(path, NULL, 0);
    anmCount = 0;
    numScripts = 0;
    spriteCount = 0;
    i = 0;
    anm = &this->anms[slot];
    if (data == NULL)
    {
        return NULL;
    }
    anm->slot = slot;
    anm->file = data;
    strcpy(anm->path, path);
    header = data;
    while (1)
    {
        anmCount++;
        numScripts += header->numScripts;
        spriteCount += header->numSprites;
        if (header->nextOffset == 0)
        {
            break;
        }
        header = (AnmFile *)((BYTE *)header + header->nextOffset);
    }
    anm->anmCount = anmCount;
    anm->textures = (AnmTexture *)g_MemoryManager.Alloc(anmCount * sizeof(AnmTexture), GLOBAL_H);
    memset(anm->textures, 0, anmCount * sizeof(AnmTexture));
    anm->sprites = (SpriteData *)g_MemoryManager.Alloc(spriteCount * sizeof(SpriteData), GLOBAL_H);
    anm->scripts = (void **)g_MemoryManager.Alloc(numScripts * sizeof(void *), GLOBAL_H);
    header = data;
    anmCount = 0;
    spriteCount = 0;
    numScripts = 0;
    while (1)
    {
        result = this->PreloadAnmTexture(anm, i, &spriteCount, &numScripts, header);
        if (result < 0)
        {
            return NULL;
        }
        i++;
        if (header->nextOffset == 0)
        {
            break;
        }
        header = (AnmFile *)((BYTE *)header + header->nextOffset);
    }
    return anm;
}

AnmLoaded *SpriteEngine::PreloadAnm(int slot, char *path)
{
    AnmLoaded *anm;

    anm = this->PreloadAnmFile(slot, path);
    if (anm == NULL)
    {
        return NULL;
    }
    anm->loadStep = 1;
    while (anm->loadStep != 0 && !g_App.closeSubThread)
    {
        Sleep(1);
    }
    return g_App.closeSubThread ? NULL : anm;
}

// 0x43c800
int SpriteEngine::LoadAnmSprites(AnmLoaded *anm, int index, int spriteOffset, int scriptOffset, AnmFile *file)
{
    SpriteData data;
    int *entries;
    int k;
    AnmSprite *rawSprite;
    char *name;
    D3DSURFACE_DESC desc;
    AnmFile *anmHeader;
    int highest;

    highest = 0;
    if (file == NULL)
    {
        LogError(&g_Log, "アニメが読み込めません。データが失われてるか壊れています\r\n");
        return -1;
    }
    anmHeader = file;
    if (anmHeader->version != 3)
    {
        LogError(&g_Log, "アニメのバージョンが違います\r\n");
        return -1;
    }
    if (!anmHeader->hasImage)
    {
        name = (char *)anmHeader + anmHeader->nameOffset;
        if (*name == '@')
        {
            this->CreateTexture(&anm->textures[index], anmHeader->width, anmHeader->height, anmHeader->format);
        }
        else if (this->LoadTexture(&anm->textures[index], anmHeader->format, anmHeader->colorKey) != 0)
        {
            LogError(&g_Log, "テクスチャ %s が作成できません。データが失われてるか壊れています\r\n", name);
            return -1;
        }
    }
    else if (this->CreateTextureFromImage(&anm->textures[index],
                                          (AnmImage *)((BYTE *)anmHeader + anmHeader->imageOffset),
                                          anmHeader->format) != 0)
    {
        LogError(&g_Log, "テクスチャが読み込めません。データが失われてるか壊れています\r\n");
        return -1;
    }
    anm->textures[index].texture->SetPriority(anmHeader->priority);
    anm->textures[index].texture->PreLoad();
    anm->textures[index].texture->GetLevelDesc(0, &desc);
    entries = anmHeader->spriteOffsets;
    for (k = 0; k < anmHeader->numSprites; k++, entries++)
    {
        rawSprite = (AnmSprite *)((char *)anmHeader + *entries);
        data.sourceFileIndex = anm->slot;
        data.texture = anm->textures[index].texture;
        data.scaleX = (float)desc.Width / anmHeader->width;
        data.scaleY = (float)desc.Height / anmHeader->height;
        data.startX = data.scaleX * rawSprite->x;
        data.startY = data.scaleY * rawSprite->y;
        data.endX = (rawSprite->x + rawSprite->width) * data.scaleX;
        data.endY = (rawSprite->y + rawSprite->height) * data.scaleY;
        data.textureWidth = desc.Width;
        data.textureHeight = desc.Height;
        anm->LoadSprite(spriteOffset, &data);
        spriteOffset++;
    }
    for (k = 0; k < anmHeader->numScripts; k++, entries += 2)
    {
        anm->scripts[scriptOffset] = (char *)anmHeader + entries[1];
        scriptOffset++;
    }
    return highest + 1;
}

void SpriteEngine::ReleaseAnm(int slot)
{
    int i;

    if (slot < 0 || slot >= ARRAY_SIZE(this->anms))
    {
        return;
    }
    if (this->anms[slot].file != NULL)
    {
        for (i = 0; i < this->anms[slot].anmCount; i++)
        {
            this->ReleaseTexture(&this->anms[slot].textures[i]);
        }
        g_MemoryManager.Free(this->anms[slot].textures);
        g_MemoryManager.Free(this->anms[slot].sprites);
        g_MemoryManager.Free(this->anms[slot].scripts);
        g_MemoryManager.Free(this->anms[slot].file);
        memset(&this->anms[slot], 0, sizeof(AnmLoaded));
    }
}

// 0x43cae0
int SpriteEngine::LoadSurface(int surface, char *path)
{
    void *data;
    int size;
    IDirect3DSurface8 *srcSurface;

    if (this->surfaces[surface] != NULL)
    {
        this->ReleaseSurface(surface);
    }
    if (this->surfaceFiles[surface] == NULL)
    {
        data = OpenPath(path, &size, 0);
        if (data == NULL)
        {
            LogError(&g_Log, "%sが読み込めないです。\r\n", path);
            return -1;
        }
    }
    else
    {
        data = this->surfaceFiles[surface];
        size = this->surfaceFileSizes[surface];
        this->surfaceFiles[surface] = NULL;
    }
    if (g_App.d3dDevice->CreateImageSurface(640, 1024, g_App.presentParams.BackBufferFormat, &srcSurface) != D3D_OK)
    {
        return -1;
    }
    if (D3DXLoadSurfaceFromFileInMemory(srcSurface, NULL, NULL, data, size, NULL, D3DX_FILTER_NONE, 0,
                                        &this->surfaceInfos[surface]) != D3D_OK)
    {
        goto fail;
    }
    if (g_App.d3dDevice->CreateRenderTarget(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat, D3DMULTISAMPLE_NONE, TRUE,
                                            &this->surfaces[surface]) != D3D_OK &&
        g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat,
                                            &this->surfaces[surface]) != D3D_OK)
    {
        goto fail;
    }
    if (g_App.d3dDevice->CreateImageSurface(this->surfaceInfos[surface].Width, this->surfaceInfos[surface].Height,
                                            g_App.presentParams.BackBufferFormat,
                                            &this->surfacesBackup[surface]) != D3D_OK)
    {
        goto fail;
    }
    if (D3DXLoadSurfaceFromSurface(this->surfaces[surface], NULL, NULL, srcSurface, NULL, NULL, D3DX_FILTER_NONE, 0) !=
        D3D_OK)
    {
        goto fail;
    }
    if (D3DXLoadSurfaceFromSurface(this->surfacesBackup[surface], NULL, NULL, srcSurface, NULL, NULL, D3DX_FILTER_NONE,
                                   0) != D3D_OK)
    {
        goto fail;
    }
    if (srcSurface != NULL)
    {
        srcSurface->Release();
        srcSurface = NULL;
    }
    g_MemoryManager.Free(data);
    return 0;

fail:
    if (srcSurface != NULL)
    {
        srcSurface->Release();
        srcSurface = NULL;
    }
    g_MemoryManager.Free(data);
    return -1;
}

int SpriteEngine::PreloadSurface(int surface, char *path)
{
    void *data;
    int size;

    if (this->surfaces[surface] != NULL)
    {
        this->ReleaseSurface(surface);
    }
    data = OpenPath(path, &size, 0);
    if (data == NULL)
    {
        LogError(&g_Log, "%sが読み込めないです。\r\n", path);
        return -1;
    }
    this->surfaceFiles[surface] = data;
    this->surfaceFileSizes[surface] = size;
    return 0;
}

AnmLoaded *SpriteEngine::PostloadAnm(AnmLoaded *anm)
{
    AnmFile *header;
    int scriptOffset;
    int result;
    AnmFile *data;
    int done;
    int spriteOffset;
    int index;

    data = anm->file;
    done = 0;
    scriptOffset = 0;
    spriteOffset = 0;
    index = 0;
    header = data;
    while (1)
    {
        if (done == anm->loadStep - 1)
        {
            result = this->LoadAnmSprites(anm, index, spriteOffset, scriptOffset, header);
            if (result < 0)
            {
                anm->loadStep = 0;
                return NULL;
            }
        }
        spriteOffset += header->numSprites;
        scriptOffset += header->numScripts;
        index++;
        if (header->nextOffset == 0)
        {
            break;
        }
        header = (AnmFile *)((BYTE *)header + header->nextOffset);
        done++;
        if (done == anm->loadStep)
        {
            anm->loadStep++;
            return anm;
        }
    }
    anm->loadStep = 0;
    return anm;
}

int SpriteEngine::StepAnmLoading()
{
    int i;

    for (i = 0; i < ARRAY_SIZE(this->anms); i++)
    {
        if (this->anms[i].releaseRequested)
        {
            this->ReleaseAnm(i);
            this->anms[i].releaseRequested = 0;
        }
        else if (this->anms[i].loadStep != 0)
        {
            if (this->PostloadAnm(&this->anms[i]) == NULL)
            {
                return -1;
            }
        }
    }
    return 0;
}

AnmLoaded *SpriteEngine::LoadAnm(int slot, char *path)
{
    AnmLoaded *anm;

    anm = this->PreloadAnmFile(slot, path);
    if (anm != NULL)
    {
        anm->loadStep = 1;
        while (anm->loadStep != 0)
        {
            anm = this->PostloadAnm(anm);
        }
    }
    return anm;
}
