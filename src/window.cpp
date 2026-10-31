// The game's window and Direct3D set-up, as th08's GameWindow object.

#include <direct.h>

#include <windows.h>
#define NO_SHDOCVW_GUIDS
#include <shlguid.h>
#include <shobjidl.h>

#include "gamemanager.h"
#include "sprite.h"
#include "unsorted.h"

double GameWindow::GetTime()
{
    if (g_GameWindow.perfFrequency[0] != 0)
    {
        // A LARGE_INTEGER, but only 4-aligned, like GameWindow's.
        DWORD count[2];

        QueryPerformanceCounter((LARGE_INTEGER *)count);
        return (double)count[0] / (double)g_GameWindow.perfFrequency[0];
    }
    else
    {
        double time;

        timeBeginPeriod(1);
        time = timeGetTime() * 0.001f;
        timeEndPeriod(1);
        return time;
    }
}

int CreateD3d()
{
    g_App.d3d = Direct3DCreate8(220);
    if (g_App.d3d == NULL)
    {
        LogError(&g_Log, "Direct3D オブジェクトは何故か作成出来なかった\r\n");
        return 1;
    }
    return 0;
}

void __stdcall ForceForegroundWindow(HWND hwnd)
{
    DWORD activeThread;
    DWORD thread;
    DWORD lockTime;

    activeThread = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
    thread = GetWindowThreadProcessId(hwnd, NULL);
    AttachThreadInput(thread, activeThread, TRUE);
    SystemParametersInfoA(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &lockTime, 0);
    SystemParametersInfoA(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, NULL, 0);
    SetActiveWindow(hwnd);
    SystemParametersInfoA(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &lockTime, 0);
    AttachThreadInput(thread, activeThread, FALSE);
}

int ResolveShortcut(char *path, char *out, int size)
{
    HRESULT res;
    int ok;
    IShellLinkA *shellLink;
    IPersistFile *storage;
    WCHAR *buf;
    WIN32_FIND_DATAA findData;

    if (out == NULL)
    {
        return 0;
    }
    ok = 0;
    CoInitialize(NULL);
    res = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkA, (void **)&shellLink);
    if (SUCCEEDED(res))
    {
        res = shellLink->QueryInterface(IID_IPersistFile, (void **)&storage);
        if (SUCCEEDED(res))
        {
            buf = new WCHAR[size];
            if (SUCCEEDED(res))
            {
                MultiByteToWideChar(CP_ACP, 0, path, -1, buf, size);
                res = storage->Load(buf, STGM_READ);
                if (SUCCEEDED(res))
                {
                    res = shellLink->GetPath(out, size, &findData, 0);
                    if (SUCCEEDED(res))
                    {
                        ok = 1;
                    }
                }
            }
            delete buf;
            storage->Release();
        }
        shellLink->Release();
    }
    CoUninitialize();
    return ok;
}

void Log::Reset()
{
    this->cursor = this->buffer;
    *this->cursor = '\0';
}

void Log::Flush()
{
    if (this->cursor != this->buffer)
    {
        LogPrintf(this, "---------------------------------------------------------- \r\n");
        if (this->hasError)
        {
            MessageBoxA(NULL, this->buffer, "log", MB_ICONERROR);
        }
        WriteToFile("./log.txt", this->buffer, strlen(this->buffer));
    }
}

void SpriteEngine::ReleaseSurfaces()
{
    int i;

    for (i = 0; i < 0x20; i++)
    {
        if (this->surfaces[i] != NULL)
        {
            this->surfaces[i]->Release();
            this->surfaces[i] = NULL;
        }
    }
}

void SpriteEngine::DoPendingCopy()
{
    if (this->pendingTexCopyTexture >= 0)
    {
        this->CopyBackBufferToTexture(this->pendingTexCopyTexture, this->pendingTexCopySrcLeft,
                                      this->pendingTexCopySrcTop, this->pendingTexCopySrcWidth,
                                      this->pendingTexCopySrcHeight, this->pendingTexCopyDstLeft,
                                      this->pendingTexCopyDstTop, this->pendingTexCopyDstWidth,
                                      this->pendingTexCopyDstHeight);
        this->pendingTexCopyTexture = -1;
    }
    if (this->pendingCopyTexture >= 0)
    {
        this->CopyBackBufferToSurface(this->pendingCopyTexture, this->pendingCopySrcLeft, this->pendingCopySrcTop,
                                      this->pendingCopySrcWidth, this->pendingCopySrcHeight, this->pendingCopyDstLeft,
                                      this->pendingCopyDstTop, this->pendingCopyDstWidth, this->pendingCopyDstHeight);
        this->pendingCopyTexture = -1;
    }
}

int ChecksumExecutable()
{
    char exePath[0x105];
    int sum;
    char *name;
    int *data;
    int idx;
    int *words;
    int fileSize;

    if (GetModuleFileNameA(NULL, exePath, sizeof(exePath)) != 0)
    {
        sum = 0;
        name = strrchr(exePath, '\\');
        if (name == NULL)
        {
            name = strrchr(exePath, '/');
        }
        if (name == NULL)
        {
            name = exePath;
        }
        else
        {
            name++;
        }
        data = words = (int *)OpenPath(name, &fileSize, 1);
        if (words == NULL)
        {
            return -1;
        }
        for (idx = 0; idx < fileSize / 4 - 1; idx++, words++)
        {
            sum += *words;
        }
        g_MemoryManager.Free(data);
        g_App.exeSize = fileSize;
        g_App.exeChecksum = sum;
        return sum;
    }
    return -1;
}

void PresentFrame()
{
    int i;
    char path[0x100];

    if (g_App.d3dDevice->Present(NULL, NULL, NULL, NULL) < 0)
    {
        g_SpriteEngine->ReleaseSurfaces();
        g_App.d3dDevice->Reset(&g_App.presentParams);
        SetDefaultRenderStates();
        g_App.redrawFrames = 2;
    }
    g_SpriteEngine->DoPendingCopy();
    if (g_PlayerInputs[2].IsJustPressed(0x800) && !(g_App.stateFlags >> 10 & 1))
    {
        _mkdir("snapshot");
        for (i = 0; i < 1000; i++)
        {
            sprintf(path, "snapshot/th%.3d.bmp", i);
            if (!FileExists(path))
            {
                break;
            }
        }
        if (i < 1000)
        {
            g_App.SaveScreenshot(path);
        }
    }
    if (g_App.redrawFrames != 0 && !g_GameManager.isPaused)
    {
        g_App.redrawFrames--;
    }
}
