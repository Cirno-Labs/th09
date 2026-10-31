// The game's window and Direct3D set-up, as th08's GameWindow object.

#include <direct.h>

// For CoInitializeEx.
#define _WIN32_DCOM

#include <windows.h>
#define NO_SHDOCVW_GUIDS
#include <shlguid.h>
#include <shobjidl.h>
#include <winnls32.h>

#include "chain.h"
#include "gamemanager.h"
#include "midi.h"
#include "network.h"
#include "sprite.h"
#include "stage.h"
#include "unsorted.h"

Network::Network()
{
    memset(this, 0, sizeof(Network));
    this->syncFrames = 60;
}

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

int CheckSingleInstance()
{
    char exePath[0x105];
    char consoleTitle[0x105];
    char *extension;

    g_AppMutex = CreateMutexA(NULL, TRUE, "Touhou 08 App");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        LogError(&g_Log, "二つは起動できません\r\n");
        return -1;
    }
    STARTUPINFOA startup = {sizeof(STARTUPINFOA)};
    GetModuleFileNameA(NULL, exePath, 0x105);
    GetConsoleTitleA(consoleTitle, 0x105);
    GetStartupInfoA(&startup);
    if (startup.lpTitle != NULL)
    {
        extension = strrchr(startup.lpTitle, '.');
        if (FileExists(startup.lpTitle) && extension != NULL)
        {
            if (_stricmp(extension, ".lnk") == 0)
            {
                do
                {
                    ResolveShortcut(startup.lpTitle, consoleTitle, 0x104);
                    extension = strrchr(consoleTitle, '.');
                } while (_stricmp(extension, ".lnk") == 0);
            }
            else
            {
                strcpy(consoleTitle, startup.lpTitle);
            }
            if (strcmp(exePath, consoleTitle) != 0)
            {
                g_GameWindow.forceClockPacing = 1;
            }
        }
        g_App.stateFlags &= ~0x40;
    }
    else
    {
        g_App.stateFlags |= 0x40;
    }
    if (g_AppMutex == NULL)
    {
        return -1;
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

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_ERASEBKGND:
        return 1;
    case WM_ACTIVATEAPP:
        g_GameWindow.isAppActive = wParam;
        g_GameWindow.isAppInactive = wParam == 0;
        break;
    case WM_CLOSE:
        g_App.stateFlags |= 0x80;
        return 1;
    case WM_SETCURSOR:
        if (!g_App.GetWindowed() && !g_GameWindow.isAppInactive)
        {
            ShowCursor(FALSE);
            SetCursor(NULL);
        }
        else
        {
            SetCursor(LoadCursorA(NULL, IDC_ARROW));
            ShowCursor(TRUE);
        }
        return 1;
    case MM_MOM_DONE:
        if (g_App.midi != NULL)
        {
            g_App.midi->ReleaseHeader((LPMIDIHDR)lParam);
        }
        break;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

// Registers the window class and creates the window, as th08's.
// Returns 0, or 1 if the window cannot be created.
static int CreateGameWindow(HINSTANCE instance)
{
    WNDCLASSA wc;
    int width;
    int height;

    memset(&wc, 0, sizeof(wc));
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hInstance = instance;
    wc.lpfnWndProc = WindowProc;
    g_GameWindow.isAppActive = 1;
    g_GameWindow.isAppInactive = 0;
    wc.lpszClassName = "BASE";
    RegisterClassA(&wc);
    if (!g_App.GetWindowed())
    {
        width = 640;
        height = 480;
        g_GameWindow.hwnd =
            CreateWindowExA(0, "BASE", "東方花映塚　～ Phantasmagoria of Flower View. ver 1.50a", WS_OVERLAPPEDWINDOW,
                            0, 0, width, height, NULL, NULL, instance, NULL);
    }
    else
    {
        width = GetSystemMetrics(SM_CXDLGFRAME) * 2 + 640;
        height = GetSystemMetrics(SM_CYDLGFRAME) * 2 + 480 + GetSystemMetrics(SM_CYCAPTION);
        g_GameWindow.hwnd = CreateWindowExA(0, "BASE", "東方花映塚　～ Phantasmagoria of Flower View. ver 1.50a",
                                            WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT,
                                            width, height, NULL, NULL, instance, NULL);
    }
    g_App.hwnd = g_GameWindow.hwnd;
    if (g_GameWindow.hwnd == NULL)
    {
        return 1;
    }
    ForceForegroundWindow(g_GameWindow.hwnd);
    return 0;
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

int GameWindow::RunFrame()
{
    int res;

    this->now = this->GetTime();
    // The clock went back: start again from now.
    if (this->lastTime > this->now)
    {
        this->nextFrameTime = this->now;
    }
    this->lastTime = this->now;
    if (this->nextFrameTime < this->now)
    {
        while (this->nextFrameTime < this->now)
        {
            this->nextFrameTime += 1.0 / 60.0;
        }
        g_SpriteEngine->FlushQuadBatch();
        g_App.SetView2D(2);
        res = g_Chain.RunCalcChain();
        g_SoundPlayer.PlayQueuedSounds();
        if (res == 0)
        {
            g_App.CloseSubThread();
            return 1;
        }
        if (res == -1)
        {
            g_App.CloseSubThread();
            return 2;
        }
        this->curFrame++;
        if (g_App.config.frameSkip < this->curFrame)
        {
            g_App.d3dDevice->BeginScene();
            g_SpriteEngine->BeginQuadBatch();
            g_App.fogState = 0xff;
            g_App.DisableFog();
            g_Chain.RunDrawChain();
            g_SpriteEngine->FlushQuadBatch();
            g_App.d3dDevice->SetTexture(0, NULL);
            g_App.d3dDevice->EndScene();
            this->curFrame = 0;
        }
        this->now = this->GetTime();
        PresentFrame();
    }
    else
    {
        Sleep(0);
    }
    return 0;
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

void SetDefaultRenderStates()
{
    float fogVal;

    g_App.d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    g_App.d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    g_App.d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    g_App.d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    g_App.d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    g_App.d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    g_App.d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_App.d3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    g_App.d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    g_App.d3dDevice->SetRenderState(D3DRS_ALPHAREF, 4);
    g_App.d3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    if (g_App.GetNoFog() == 0)
    {
        g_App.d3dDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
    }
    else
    {
        g_App.d3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    }
    fogVal = 1.0f;
    g_App.d3dDevice->SetRenderState(D3DRS_FOGDENSITY, *(DWORD *)&fogVal);
    g_App.d3dDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
    g_App.d3dDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
    g_App.d3dDevice->SetRenderState(D3DRS_FOGCOLOR, 0xffa0a0a0);
    fogVal = 1000.0f;
    g_App.d3dDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fogVal);
    fogVal = 5000.0f;
    g_App.d3dDevice->SetRenderState(D3DRS_FOGEND, *(DWORD *)&fogVal);
    // Always true: | where & was meant, as th08's.
    if (g_App.caps.RasterCaps | D3DPRASTERCAPS_ANTIALIASEDGES)
    {
        g_App.d3dDevice->SetRenderState(D3DRS_EDGEANTIALIAS, FALSE);
    }
    g_App.d3dDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSW, D3DTADDRESS_CLAMP);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
    if (g_SpriteEngine != NULL)
    {
        g_SpriteEngine->ClearBlendMode();
        g_SpriteEngine->ClearColorOp();
        g_SpriteEngine->ClearVertexFormat();
        g_SpriteEngine->ClearTexture();
        g_SpriteEngine->ClearLastCameraSetupFlag();
    }
    if (g_GameManager.sides[0].stage != NULL)
    {
        g_GameManager.sides[0].stage->fogColorChanged = 1;
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int runResult = 0;
    MSG message;
    HRESULT testResult;
    int j;
    int i;

    // The data are read next to the executable.
    if (GetModuleFileNameA(NULL, g_DataDir, 0x104) != 0)
    {
        for (i = strlen(g_DataDir) - 1; g_DataDir[i] != '\\' && g_DataDir[i] != '/'; i--)
        {
            if (i <= 0)
            {
                break;
            }
        }
        if (i <= 0)
        {
            strcpy(g_DataDir, "./");
        }
        else
        {
            g_DataDir[i] = '/';
            g_DataDir[i + 1] = '\0';
        }
    }
    else
    {
        strcpy(g_DataDir, "./");
    }
    g_App.hInstance = hInstance;
    SystemParametersInfoA(SPI_GETSCREENSAVEACTIVE, 0, &g_GameWindow.screenSaveActive, 0);
    SystemParametersInfoA(SPI_GETLOWPOWERACTIVE, 0, &g_GameWindow.lowPowerActive, 0);
    SystemParametersInfoA(SPI_GETPOWEROFFACTIVE, 0, &g_GameWindow.powerOffActive, 0);
    SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, 0, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETLOWPOWERACTIVE, 0, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETPOWEROFFACTIVE, 0, NULL, SPIF_SENDCHANGE);
    g_App.InitCriticalSections();
    LogPrintf(&g_Log, "東方動作記録 --------------------------------------------- \r\n");
    if (CheckSingleInstance() == -1)
    {
        goto stop;
    }
    if (g_App.LoadConfig("th09.cfg") != 0)
    {
        goto stop;
    }
    ChecksumExecutable();
    QueryPerformanceFrequency((LARGE_INTEGER *)g_GameWindow.perfFrequency);
restart:
    if (CreateD3d() != 0)
    {
        goto stop;
    }
    if (CreateGameWindow(hInstance) != 0)
    {
        goto stop;
    }
    if (InitD3dDevice() != 0)
    {
        goto stop;
    }
    g_SoundPlayer.Initialize(g_GameWindow.hwnd);
    InitJoystick();
    ResetKeyboard();
    g_SpriteEngine =
        (SpriteEngine *)g_MemoryManager.Register(new SpriteEngine(), sizeof(SpriteEngine), "SprtCtrlInf");
    g_Rng.SetSeed(timeGetTime());
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    g_Network = (Network *)g_MemoryManager.Register(new Network(), sizeof(Network), "NetworkInf");
    g_Network->Init();
    if (g_Network->CreatePeer() != 0)
    {
        LogError(&g_Log, "DirectPlay 初期化失敗\r\n");
    }
    if (!g_App.GetWindowed())
    {
        WINNLSEnableIME(NULL, FALSE);
        ShowCursor(FALSE);
        SetCursor(NULL);
    }
    runResult = App::RegisterChain();
    if (runResult != 0)
    {
        if (runResult == -1)
        {
            goto release;
        }
        runResult = 2;
        goto release;
    }
    runResult = 0;
    g_GameWindow.curFrame = -4;
    g_GameWindow.now = g_GameWindow.lastTime = g_GameWindow.nextFrameTime = 0;
    while (!g_GameWindow.isClosing)
    {
        if (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        else
        {
            testResult = g_App.d3dDevice->TestCooperativeLevel();
            if (testResult == D3D_OK)
            {
                runResult = g_GameWindow.RunFrame();
                if (runResult != 0)
                {
                    break;
                }
                g_App.stateFlags &= ~0x10;
            }
            else if (testResult == D3DERR_DEVICENOTRESET)
            {
                g_SpriteEngine->ReleaseSurfaces();
                if (g_App.d3dDevice->Reset(&g_App.presentParams) != 0)
                {
                    break;
                }
                SetDefaultRenderStates();
                g_App.redrawFrames = 3;
                g_App.stateFlags |= 0x10;
            }
        }
    }
release:
    g_Chain.Release();
    while (g_SoundPlayer.PlayQueuedSounds())
    {
    }
stop:
    Sleep(1000);
    g_SoundPlayer.Release();
    delete g_SpriteEngine;
    g_SpriteEngine = NULL;
    if (g_App.d3dDevice != NULL)
    {
        g_App.d3dDevice->Reset(&g_App.presentParams);
    }
    if (g_App.d3dDevice != NULL)
    {
        g_App.d3dDevice->Release();
        g_App.d3dDevice = NULL;
    }
    if (g_App.d3d != NULL)
    {
        g_App.d3d->Release();
        g_App.d3d = NULL;
    }
    if (g_Network != NULL)
    {
        g_Network->Close();
        g_Network->ReleaseInterfaces();
        delete g_Network;
        g_Network = NULL;
    }
    if (g_GameWindow.hwnd != NULL)
    {
        ShowWindow(g_GameWindow.hwnd, SW_HIDE);
        MoveWindow(g_GameWindow.hwnd, 0, 0, 0, 0, FALSE);
        DestroyWindow(g_GameWindow.hwnd);
        g_GameWindow.hwnd = NULL;
    }
    ShowCursor(TRUE);
    if (runResult == 2)
    {
        g_Log.Reset();
        LogPrintf(&g_Log, "再起動を要するオプションが変更されたので再起動します\r\n");
        if (!g_App.GetWindowed())
        {
            WINNLSEnableIME(NULL, TRUE);
        }
        j = 0;
        while (j < 60)
        {
            if (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            }
            j++;
        }
        goto restart;
    }
    WriteToFile("th09.cfg", &g_App.config, sizeof(g_App.config));
    if (g_App.midi != NULL)
    {
        g_App.midi->Stop();
        delete g_App.midi;
        g_App.midi = NULL;
    }
    g_Log.Flush();
    g_App.DeleteCriticalSections();
    CoUninitialize();
    SystemParametersInfoA(SPI_SETSCREENSAVEACTIVE, g_GameWindow.screenSaveActive, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETLOWPOWERACTIVE, g_GameWindow.lowPowerActive, NULL, SPIF_SENDCHANGE);
    SystemParametersInfoA(SPI_SETPOWEROFFACTIVE, g_GameWindow.powerOffActive, NULL, SPIF_SENDCHANGE);
    WINNLSEnableIME(NULL, TRUE);
    return 0;
}
