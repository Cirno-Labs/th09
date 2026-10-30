#include "midi.h"
#include "screeneffect.h"
#include "sprite.h"
#include "timer.h"
#include "unsorted.h"

int App::GetNoFog()
{
    return this->flags.noFog;
}

int App::GetNoScore()
{
    return this->stateFlags >> 3 & 1;
}

int App::GetForce16BitTextures()
{
    return this->flags.force16BitTextures;
}

int App::GetReferenceRasterizer()
{
    return this->flags.referenceRasterizer;
}

int App::GetWindowed()
{
    return this->windowed;
}

int App::GetNoDirectInput()
{
    return this->flags.noDirectInput;
}

int App::GetPreloadBgm()
{
    return this->flags.preloadBgm;
}

int App::GetNoVsync()
{
    return this->flags.noVsync;
}

int App::GetNoTextRenderDetect()
{
    return this->flags.noTextRenderDetect;
}

int SpriteEngine::SetPendingSurfaceCopy(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight,
                                        int dstLeft, int dstTop, int dstWidth, int dstHeight)
{
    if (this->pendingCopyTexture >= 0)
    {
        return -1;
    }
    this->pendingCopyTexture = texture;
    this->pendingCopySrcLeft = srcLeft;
    this->pendingCopySrcTop = srcTop;
    this->pendingCopySrcWidth = srcWidth;
    this->pendingCopySrcHeight = srcHeight;
    this->pendingCopyDstLeft = dstLeft;
    this->pendingCopyDstTop = dstTop;
    this->pendingCopyDstWidth = dstWidth;
    this->pendingCopyDstHeight = dstHeight;
    return 0;
}

int App::LoadBgm(int id)
{
    char *dot;
    char wavFile[0x100];
    DWORD i;

    for (i = 0; i < ARRAY_SIZE(g_BgmInfos); i++)
    {
        if (g_BgmInfos[i].id == id)
        {
            break;
        }
    }
    if (i >= ARRAY_SIZE(g_BgmInfos))
    {
        return 1;
    }
    if (g_App.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            g_App.midi->LoadFile(g_BgmInfos[i].id, g_BgmInfos[i].path);
        }
        return 0;
    }
    if (g_App.musicMode == 1)
    {
        strcpy(wavFile, g_BgmInfos[i].path);
        dot = strrchr(wavFile, '.');
        dot[1] = 'w';
        dot[2] = 'a';
        dot[3] = 'v';
        g_SoundPlayer.QueueSoundCommand(1, g_BgmInfos[i].id, wavFile);
    }
    return 1;
}

int App::PlayBgm()
{
    if (g_App.musicMode == 2)
    {
        if (g_App.midi != NULL)
        {
            g_App.midi->Play();
        }
    }
    else if (g_App.musicMode == 1)
    {
        g_SoundPlayer.QueueSoundCommand(10, 0, "dummy");
    }
    return 0;
}

void Timer::Add(float frames)
{
    this->previous = this->current;
    if (g_App.gameSpeed > 0.99f)
    {
        this->currentF += frames;
    }
    else
    {
        this->currentF += g_App.gameSpeed * frames;
    }
    this->current = this->currentF;
    if (g_App.stateFlags & 0x20)
    {
        this->previous = -999999;
        this->currentF += this->current;
    }
}

void App::TickTimer(int *frames, float *subFrame)
{
    if (this->gameSpeed <= 0.99f)
    {
        *subFrame += this->gameSpeed;
        *frames = *subFrame;
    }
    else
    {
        (*frames)++;
        *subFrame += 1.0f;
    }
}

void SpriteEngine::ResetFrameState()
{
    this->ClearVertexFormat();
    this->ClearSpriteData();
    this->ClearTexture();
    this->ClearColorOp();
    this->ClearBlendMode();
    this->ClearZWrite();
    this->ClearDrawStats();
    this->ClearLastCameraSetupFlag();
    this->ResetColorModulation();
    this->shakeY = 0.0f;
    this->shakeX = 0.0f;
}

int App::EnableFog()
{
    g_SpriteEngine->FlushQuadBatch();
    if (this->fogState != 1)
    {
        this->fogState = 1;
        return this->d3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
    }
    return 0;
}

int App::DisableFog()
{
    g_SpriteEngine->FlushQuadBatch();
    if (this->fogState != 0)
    {
        this->fogState = 0;
        return this->d3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    }
    return 0;
}

int App::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
    g_SpriteEngine->FlushQuadBatch();
    return this->d3dDevice->SetRenderState(state, value);
}

void App::AddPlayTime()
{
    DWORD now = timeGetTime();
    DWORD delta;

    if (now < this->lastPlayTimeTick)
    {
        this->lastPlayTimeTick = now;
    }
    delta = now - this->lastPlayTimeTick;
    g_RunTime.hours += delta / 3600000;
    delta %= 3600000;
    g_RunTime.minutes += delta / 60000;
    delta %= 60000;
    g_RunTime.seconds += delta / 1000;
    delta %= 1000;
    g_RunTime.milliseconds += delta;
    if (g_RunTime.milliseconds >= 1000)
    {
        g_RunTime.seconds += g_RunTime.milliseconds / 1000;
        g_RunTime.milliseconds %= 1000;
    }
    if (g_RunTime.seconds >= 60)
    {
        g_RunTime.minutes += g_RunTime.seconds / 60;
        g_RunTime.seconds %= 60;
    }
    if (g_RunTime.minutes >= 60)
    {
        g_RunTime.hours += g_RunTime.minutes / 60;
        g_RunTime.minutes %= 60;
    }
    this->lastPlayTimeTick = now;
}

void App::AddTotalPlayTime()
{
    DWORD now = timeGetTime();
    DWORD delta;

    if (now < this->lastTotalPlayTimeTick)
    {
        this->lastTotalPlayTimeTick = 0;
    }
    delta = now - this->lastTotalPlayTimeTick;
    g_PlayTime.hours += delta / 3600000;
    delta %= 3600000;
    g_PlayTime.minutes += delta / 60000;
    delta %= 60000;
    g_PlayTime.seconds += delta / 1000;
    delta %= 1000;
    g_PlayTime.milliseconds += delta;
    if (g_PlayTime.milliseconds >= 1000)
    {
        g_PlayTime.seconds += g_PlayTime.milliseconds / 1000;
        g_PlayTime.milliseconds %= 1000;
    }
    if (g_PlayTime.seconds >= 60)
    {
        g_PlayTime.minutes += g_PlayTime.seconds / 60;
        g_PlayTime.seconds %= 60;
    }
    if (g_PlayTime.minutes >= 60)
    {
        g_PlayTime.hours += g_PlayTime.minutes / 60;
        g_PlayTime.minutes %= 60;
    }
    this->lastTotalPlayTimeTick = now;
}

int App::CheckVersionFile(char *version, DWORD size, DWORD checksum)
{
    DWORD size2;
    char *start;
    DWORD left;
    DWORD checksum2;
    char *p;

    if (this->verFile == NULL)
    {
        return 0;
    }
    p = this->verFile;
    left = this->verFileSize;
    if (strncmp(version, "debug", 5) == 0)
    {
        return 0;
    }
    if (strcmp("0150a", "debug") == 0)
    {
        return 0;
    }
    while (left > 0)
    {
        if (strncmp(version, p, 5) == 0)
        {
            p += 6;
            sscanf(p, "%d %d", &size2, &checksum2);
            if (size2 == size && checksum2 == checksum)
            {
                return 0;
            }
        }
        start = p;
        p = strchr(p, '\n');
        if (p == NULL)
        {
            break;
        }
        p++;
        left -= p - start;
    }
    return -1;
}

void App::CloseSubThread()
{
    if (this->subThread != NULL)
    {
        this->closeSubThread = 1;
        while (WaitForSingleObject(this->subThread, 1000) == WAIT_TIMEOUT)
        {
            Sleep(1);
        }
        CloseHandle(this->subThread);
        this->subThread = NULL;
        this->closeSubThread = 0;
    }
}

void App::StartLoadingSprites(D3DXVECTOR3 *pos)
{
    if (this->loadingState == 0)
    {
        this->loadingAnm->StartScriptAtOrigin(&g_LoadingSprites[0], 0);
        this->loadingAnm->StartScriptAtOrigin(&g_LoadingSprites[1], 1);
        this->loadingAnm->StartScriptAtOrigin(&g_LoadingSprites[2], 2);
        this->loadingState = 1;
        g_LoadingSprites[0].pos = *pos;
        g_LoadingSprites[1].pos = *pos;
        g_LoadingSprites[2].pos = *pos;
    }
}

void App::StopLoadingSprites()
{
    if (this->loadingState == 1)
    {
        g_LoadingSprites[0].SetPendingInterrupt(1);
        g_LoadingSprites[1].SetPendingInterrupt(1);
        g_LoadingSprites[2].SetPendingInterrupt(1);
        this->loadingState = 0;
    }
    if (g_LoadingScreenEffect != NULL)
    {
        g_LoadingScreenEffect->Finish();
        g_LoadingScreenEffect = NULL;
    }
}

void App::StartLoadingSpritesOverScreen(D3DXVECTOR3 *pos)
{
    if (this->loadingState == 0)
    {
        this->loadingAnm->StartScriptAtOrigin(&g_LoadingSprites[0], 0);
        this->loadingAnm->StartScriptAtOrigin(&g_LoadingSprites[1], 1);
        this->loadingAnm->StartScriptAtOrigin(&g_LoadingSprites[2], 2);
        this->loadingState = 1;
        g_LoadingSprites[0].pos = *pos;
        g_LoadingSprites[1].pos = *pos;
        g_LoadingSprites[2].pos = *pos;
    }
    g_SpriteEngine->SetPendingSurfaceCopy(8, 0, 0, 640, 480, 0, 0, 640, 480);
}

void App::FinishLoadingSprites()
{
    if (this->loadingState == 1)
    {
        g_LoadingSprites[0].SetPendingInterrupt(2);
        g_LoadingSprites[1].SetPendingInterrupt(2);
        g_LoadingSprites[2].SetPendingInterrupt(2);
        this->loadingState = 2;
    }
    if (g_LoadingScreenEffect != NULL)
    {
        g_LoadingScreenEffect->Finish();
        g_LoadingScreenEffect = NULL;
    }
}

void App::StartLoadingScreenEffect(int type)
{
    if (g_LoadingScreenEffect == NULL)
    {
        g_LoadingScreenEffect = ScreenEffect::StartEffect(type + 5, 60, 0, 0, 0, 1, 2);
    }
}

void App::InitCriticalSections()
{
    DWORD i;

    for (i = 0; i < ARRAY_SIZE(this->criticalSections); i++)
    {
        InitializeCriticalSection(&this->criticalSections[i]);
    }
}

void App::DeleteCriticalSections()
{
    DWORD i;

    for (i = 0; i < ARRAY_SIZE(this->criticalSections); i++)
    {
        DeleteCriticalSection(&this->criticalSections[i]);
    }
}
