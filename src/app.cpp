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
