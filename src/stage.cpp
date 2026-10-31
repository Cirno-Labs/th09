#include "stage.h"
#include "gamemanager.h"
#include "screeneffect.h"
#include "unsorted.h"

// The cubic Hermite curve from p0 to p1, with tangents m0 and m1, at `t`
// (0 to 1), as th08's. Static: its callers keep ecx, edx and esi across
// it (0x401000).
static float HermiteInterpolate(float p0, float p1, float m0, float m1, float t)
{
    float f00;
    float f10;
    float f01;
    float f11;

    f00 = (t - 1.0f) * (t - 1.0f) * (2.0f * t + 1.0f);
    f01 = t * t * (3.0f - 2.0f * t);
    f10 = (1.0f - t) * (1.0f - t) * t;
    f11 = (t - 1.0f) * t * t;
    return f00 * p0 + f01 * p1 + f10 * m0 + f11 * m1;
}

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

// Moves `out` from `from` to `to` over stage->cameraFadeFrames[i] frames,
// on stage->cameraFadeTimer[i], eased by stage->cameraFadeMode[i]: 1 to 3
// slow down, 4 to 6 speed up, 7 follows a Hermite curve with tangents m0
// and m1. Clears the frame count once the timer reaches it. As th08's;
// static, taking the stage in ecx, `i` in eax and `out` in edi (0x4016e0).
// Not matching yet: the original picks eax and ecx where this picks edx.
static void UpdateEasedVector(Stage *stage, D3DXVECTOR3 *out, int i, D3DXVECTOR3 *from, D3DXVECTOR3 *to,
                              D3DXVECTOR3 *m0, D3DXVECTOR3 *m1)
{
    float t;

    if (stage->cameraFadeTimer[i] < stage->cameraFadeFrames[i])
    {
        stage->cameraFadeTimer[i]++;
        t = stage->cameraFadeTimer[i].AsFloat() / stage->cameraFadeFrames[i];
    }
    else
    {
        stage->cameraFadeTimer[i] = stage->cameraFadeFrames[i];
        t = 1.0f;
        stage->cameraFadeFrames[i] = 0;
    }
    switch (stage->cameraFadeMode[i])
    {
    case 1:
        t = 1.0f - (t = 1.0f - t) * t;
        break;
    case 2:
        t = 1.0f - (t = 1.0f - t) * t * t;
        break;
    case 3:
        t = 1.0f - (t = 1.0f - t) * t * t * t;
        break;
    case 4:
        t = t * t;
        break;
    case 5:
        t = t * t * t;
        break;
    case 6:
        t = t * t * t * t;
        break;
    }
    if (stage->cameraFadeMode[i] != 7)
    {
        *out = *to - *from;
        *out = *out * t + *from;
    }
    else
    {
        out->x = HermiteInterpolate(from->x, to->x, m0->x, m1->x, t);
        out->y = HermiteInterpolate(from->y, to->y, m0->y, m1->y, t);
        out->z = HermiteInterpolate(from->z, to->z, m0->z, m1->z, t);
    }
}

void Stage::JumpToWaitLabel()
{
    StageInstr *cmd;
    int idx;

    if (this->waitLabel != 0)
    {
        idx = 0;
        cmd = this->script;
        this->scriptIndex = 0;
        while ((cmd->opcode != 0x1f || this->waitLabel != *(int *)&cmd->args.x) && cmd->frame != -1)
        {
            cmd++;
            idx++;
        }
        if (cmd->frame != -1)
        {
            this->scriptIndex = idx + 1;
            this->scriptTime = cmd->frame;
            this->waitLabel = 0;
        }
    }
}

void Stage::RunScript()
{
    StageInstr *cmd;
    D3DVECTOR tmpVec;
    int frame;

loop:
    cmd = &this->script[this->scriptIndex];
    frame = cmd->frame;
    if (this->scriptTime >= frame && frame != -1)
    {
        switch (cmd->opcode)
        {
        case 0:
            tmpVec = cmd->args;
            this->cameraPos.x = tmpVec.x;
            this->cameraPos.y = tmpVec.y;
            this->cameraPos.z = tmpVec.z;
            this->keyPos = tmpVec;
            this->keyFrame = cmd->frame;
            cmd++;
            this->nextKeyFrame = cmd->frame;
            this->nextKeyPos = cmd->args;
            break;
        case 1:
            this->fog.color = *(D3DCOLOR *)&cmd->args.x;
            this->fog.nearPlane = cmd->args.y;
            this->fog.farPlane = cmd->args.z;
            this->fogTarget = this->fog;
            break;
        case 2:
            this->fogStart = this->fog;
            this->fogFadeFrames = *(int *)&cmd->args.x;
            this->fogFadeTime = 0;
            break;
        case 5:
            if (this->cameraOffsetChanged)
            {
                this->cameraOffsetChanged = 0;
            }
            this->cameraTo.eye = this->cameraFrom.eye;
            this->cameraFrom.eye = cmd->args;
            if (this->cameraFadeFrames[0] == 0)
            {
                g_App.cameras[this->sideIdx].eye = cmd->args;
            }
            break;
        case 6:
            this->cameraFadeFrames[0] = *(int *)&cmd->args.x;
            this->cameraFadeTimer[0] = 0;
            this->cameraFadeMode[0] = *(int *)&cmd->args.y;
            break;
        case 7:
            this->cameraTo.at = this->cameraFrom.at;
            this->cameraFrom.at = cmd->args;
            if (this->cameraFadeFrames[1] == 0)
            {
                g_App.cameras[this->sideIdx].at = cmd->args;
            }
            break;
        case 8:
            this->cameraFadeFrames[1] = *(int *)&cmd->args.x;
            this->cameraFadeTimer[1] = 0;
            this->cameraFadeMode[1] = *(int *)&cmd->args.y;
            break;
        case 9:
            this->cameraTo.up = this->cameraFrom.up;
            this->cameraFrom.up = cmd->args;
            if (this->cameraFadeFrames[2] == 0)
            {
                g_App.cameras[this->sideIdx].up = cmd->args;
            }
            break;
        case 10:
            this->cameraFadeFrames[2] = *(int *)&cmd->args.x;
            this->cameraFadeMode[2] = *(int *)&cmd->args.y;
            this->cameraFadeTimer[2] = 0;
            break;
        case 11:
            this->cameraTo.fov = this->cameraFrom.fov;
            this->cameraFrom.fov = cmd->args.x;
            if (this->cameraFadeFrames[3] == 0)
            {
                g_App.cameras[this->sideIdx].fov = cmd->args.x;
            }
            break;
        case 12:
            this->cameraFadeFrames[3] = *(int *)&cmd->args.x;
            this->cameraFadeTimer[3] = 0;
            this->cameraFadeMode[3] = *(int *)&cmd->args.y;
            break;
        case 13:
            this->clearColor = *(D3DCOLOR *)&cmd->args.x;
            break;
        case 3:
            if (this->waitLabel != 0)
            {
                this->waitLabel = 0;
                break;
            }
            goto done;
        case 4:
            this->scriptIndex = *(int *)&cmd->args.x;
            this->scriptTime = *(int *)&cmd->args.y;
            this->cameraFadeFrames[0] = 0;
            this->cameraOffsetChanged = 1;
            goto done;
        case 14:
            this->cameraTo.eye = cmd->args;
            break;
        case 15:
            this->cameraFrom.eye = cmd->args;
            break;
        case 16:
            this->cameraFadeStart.eye = cmd->args;
            break;
        case 17:
            this->cameraFadeTarget.eye = cmd->args;
            break;
        case 18:
            this->cameraFadeFrames[0] = *(int *)&cmd->args.x;
            this->cameraFadeTimer[0] = 0;
            this->cameraFadeMode[0] = 7;
            break;
        case 19:
            this->cameraTo.at = cmd->args;
            break;
        case 20:
            this->cameraFrom.at = cmd->args;
            break;
        case 21:
            this->cameraFadeStart.at = cmd->args;
            break;
        case 22:
            this->cameraFadeTarget.at = cmd->args;
            break;
        case 23:
            this->cameraFadeFrames[1] = *(int *)&cmd->args.x;
            this->cameraFadeTimer[1] = 0;
            this->cameraFadeMode[1] = 7;
            break;
        case 24:
            this->cameraTo.up = cmd->args;
            break;
        case 25:
            this->cameraFrom.up = cmd->args;
            break;
        case 26:
            this->cameraFadeStart.up = cmd->args;
            break;
        case 27:
            this->cameraFadeTarget.up = cmd->args;
            break;
        case 28:
            this->cameraFadeFrames[2] = *(int *)&cmd->args.x;
            this->cameraFadeTimer[2] = 0;
            this->cameraFadeMode[2] = 7;
            break;
        case 29:
            if (*(int *)&cmd->args.x >= 0)
            {
                this->anm->StartScriptAtOrigin(&this->backgroundLayer0, *(int *)&cmd->args.x);
            }
            else
            {
                this->backgroundLayer0.spriteIndex = -1;
            }
            break;
        case 30:
            // Hides backgroundLayer0, not backgroundLayer1, for a negative
            // script, as th08's.
            if (*(int *)&cmd->args.x >= 0)
            {
                this->anm->StartScriptAtOrigin(&this->backgroundLayer1, *(int *)&cmd->args.x);
            }
            else
            {
                this->backgroundLayer0.spriteIndex = -1;
            }
            break;
        case 33:
            this->shakeMode = *(BYTE *)&cmd->args.x;
            this->cameraFadeFrames[4] = 0;
            this->cameraFadeTimer[4] = 0;
            this->cameraFadeMode[4] = 0;
            break;
        case 32:
            g_App.cameras[this->sideIdx].offset = cmd->args;
            break;
        case 34:
            if (*(int *)&cmd->args.x >= 0)
            {
                this->anm->StartScriptAtOrigin(&this->backgroundLayer2, *(int *)&cmd->args.x);
            }
            else
            {
                this->backgroundLayer2.spriteIndex = -1;
            }
            break;
        }
        this->scriptIndex++;
        goto loop;
    }
done:
    {
        int i;
        float delta;
        float t;
        // The fov fade's frames, read once.
        int frames;

        i = 0;
        if (this->cameraFadeFrames[i] != 0)
        {
            UpdateEasedVector(this, &g_App.cameras[this->sideIdx].eye, i, (D3DXVECTOR3 *)&this->cameraTo.eye,
                              (D3DXVECTOR3 *)&this->cameraFrom.eye, (D3DXVECTOR3 *)&this->cameraFadeStart.eye,
                              (D3DXVECTOR3 *)&this->cameraFadeTarget.eye);
        }
        i = 1;
        if (this->cameraFadeFrames[i] != 0)
        {
            UpdateEasedVector(this, &g_App.cameras[this->sideIdx].at, i, (D3DXVECTOR3 *)&this->cameraTo.at,
                              (D3DXVECTOR3 *)&this->cameraFrom.at, (D3DXVECTOR3 *)&this->cameraFadeStart.at,
                              (D3DXVECTOR3 *)&this->cameraFadeTarget.at);
        }
        i = 2;
        if (this->cameraFadeFrames[i] != 0)
        {
            UpdateEasedVector(this, &g_App.cameras[this->sideIdx].up, i, (D3DXVECTOR3 *)&this->cameraTo.up,
                              (D3DXVECTOR3 *)&this->cameraFrom.up, (D3DXVECTOR3 *)&this->cameraFadeStart.up,
                              (D3DXVECTOR3 *)&this->cameraFadeTarget.up);
        }
        i = 3;
        frames = this->cameraFadeFrames[i];
        if (frames != 0)
        {
            if (this->cameraFadeTimer[i] < frames)
            {
                this->cameraFadeTimer[i]++;
                t = this->cameraFadeTimer[i].AsFloat() / this->cameraFadeFrames[i];
            }
            else
            {
                this->cameraFadeTimer[i] = frames;
                t = 1.0f;
                this->cameraFadeFrames[i] = 0;
            }
            switch (this->cameraFadeMode[i])
            {
            case 1:
                t = 1.0f - (t = 1.0f - t) * t;
                break;
            case 2:
                t = 1.0f - (t = 1.0f - t) * t * t;
                break;
            case 3:
                t = 1.0f - (t = 1.0f - t) * t * t * t;
                break;
            case 4:
                t = t * t;
                break;
            case 5:
                t = t * t * t;
                break;
            case 6:
                t = t * t * t * t;
                break;
            }
            g_App.cameras[this->sideIdx].fov =
                (delta = this->cameraFrom.fov - this->cameraTo.fov) * t + this->cameraTo.fov;
        }
    }
    D3DXVec3Normalize(&g_App.cameras[this->sideIdx].dir, &g_App.cameras[this->sideIdx].at);
    if (this->shakeMode)
    {
        switch (this->shakeMode)
        {
        case 1:
            g_App.cameras[this->sideIdx].offset.x =
                sinf(this->cameraFadeTimer[4].AsFloat() * (3.1415927f * 2.0f / 480.0f) - 3.1415927f) * 40.0f;
            this->cameraFadeTimer[4]++;
            if (this->cameraFadeTimer[4] >= 480)
            {
                this->cameraFadeTimer[4] = 0;
            }
            break;
        case 2:
        {
            float angle;

            angle = this->cameraFadeTimer[4].AsFloat() * (3.1415927f * 2.0f / 480.0f) - 3.1415927f;
            g_App.cameras[this->sideIdx].offset.x = 70.0f * sinf(angle);
            g_App.cameras[this->sideIdx].up.x = -sinf(angle) * 0.1f;
            this->cameraFadeTimer[4]++;
            if (this->cameraFadeTimer[4] >= 480)
            {
                this->cameraFadeTimer[4] = 0;
            }
            break;
        }
        // Case 4 comes first, as the original's.
        case 4:
        {
            float angle;

            angle = this->cameraFadeTimer[4].AsFloat() * (3.1415927f * 2.0f / 2048.0f) - 3.1415927f;
            g_App.cameras[this->sideIdx].offset.x = sinf(angle) * 30.0f;
            g_App.cameras[this->sideIdx].offset.z = cosf(angle) * 30.0f;
            g_App.cameras[this->sideIdx].up.x = -sinf(angle) * 0.1f;
            this->cameraFadeTimer[4]++;
            if (this->cameraFadeTimer[4] >= 2048)
            {
                this->cameraFadeTimer[4] = 0;
            }
            break;
        }
        case 3:
        {
            float angle;

            angle = this->cameraFadeTimer[4].AsFloat() * (3.1415927f * 2.0f / 4800.0f) - 3.1415927f;
            g_App.cameras[this->sideIdx].up.x = sinf(angle);
            g_App.cameras[this->sideIdx].up.z = cosf(angle);
            this->cameraFadeTimer[4]++;
            if (this->cameraFadeTimer[4] >= 4800)
            {
                this->cameraFadeTimer[4] = 0;
            }
            break;
        }
        }
    }
    if (this->fogFadeFrames != 0)
    {
        float fogT;
        int j;

        this->fogFadeTime++;
        fogT = this->fogFadeTime.AsFloat() / this->fogFadeFrames;
        if (fogT >= 1.0f)
        {
            fogT = 1.0f;
        }
        for (j = 0; j < 4; j++)
        {
            ((BYTE *)&this->fog.color)[j] =
                ((float)((BYTE *)&this->fogTarget.color)[j] - (float)((BYTE *)&this->fogStart.color)[j]) * fogT +
                (float)((BYTE *)&this->fogStart.color)[j];
        }
        this->fog.nearPlane = (this->fogTarget.nearPlane - this->fogStart.nearPlane) * fogT + this->fogStart.nearPlane;
        this->fog.farPlane = (this->fogTarget.farPlane - this->fogStart.farPlane) * fogT + this->fogStart.farPlane;
        if (this->fogFadeTime >= this->fogFadeFrames)
        {
            this->fogFadeFrames = 0;
        }
    }
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
        color = stage->fog.color;
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
        DrawColorRect(area, stage->fog.color);
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
        g_App.SetRenderState(D3DRS_FOGCOLOR, stage->fog.color);
    }
    else
    {
        color = stage->fog.color;
        MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 2);
        MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 1);
        MODULATE_CHANNEL(color, g_SpriteEngine->modulateColor, 0);
        g_App.SetRenderState(D3DRS_FOGCOLOR, color);
    }
    g_App.SetRenderState(D3DRS_FOGSTART, *(DWORD *)&stage->fog.nearPlane);
    g_App.SetRenderState(D3DRS_FOGEND, *(DWORD *)&stage->fog.farPlane);
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

int Stage::OnDraw2(Stage *stage)
{
    float fog;
    int i;

    g_App.SetView(stage->sideIdx);
    if (stage->spellCardState <= 1)
    {
        stage->DrawLayer(2);
        stage->DrawLayer(3);
        if (!g_App.GetNoFog())
        {
            g_App.DisableFog();
        }
    }
    if (stage->tintColor.a > 0)
    {
        g_SpriteEngine->SetColorModulation(stage->tintColor.color);
    }
    stage->tintColor.a = 0;
    stage->tintColor.r = 0x80;
    stage->tintColor.g = 0x80;
    stage->tintColor.b = 0x80;
    g_SpriteEngine->FlushQuadBatch();
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    if (!g_App.GetNoFog())
    {
        g_App.DisableFog();
    }
    if (stage->spellCardState >= 1)
    {
        for (i = 0; i < stage->dialogueSpriteCount; i++)
        {
            stage->dialogueSprites[i].pos.x = g_GameManager.ArenaToScreenX(stage->dialogueSprites[i].posOffset.x);
            stage->dialogueSprites[i].pos.y = g_GameManager.ArenaToScreenY(stage->dialogueSprites[i].posOffset.y);
            stage->dialogueSprites[i].pos.z = stage->dialogueSprites[i].posOffset.z;
            g_SpriteEngine->DrawAndFlush(&stage->dialogueSprites[i]);
        }
        if (stage->spellCardDrawCallback != NULL)
        {
            stage->spellCardDrawCallback();
        }
    }
    g_SpriteEngine->SetLastCameraSetupFlag(0);
    g_App.ResetView();
    fog = 1000.0f;
    g_App.SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fog);
    fog = 2000.0f;
    g_App.SetRenderState(D3DRS_FOGEND, *(DWORD *)&fog);
    if (stage->keepColorModulation == 0)
    {
        g_SpriteEngine->ResetColorModulation();
    }
    stage->keepColorModulation = 0;
    stage->unknown6440 = 0;
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

void Stage::StartSpellCard()
{
    this->spellCardState = 1;
    this->unknownCd0 = 0;
}
