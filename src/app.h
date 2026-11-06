// The application: window, Direct3D and the main loop.
// Included by unsorted.h where these declarations were; it relies on what
// unsorted.h declares before it.

#pragma once

struct App
{
    // Initializes cameras and config, then clears the object (0x431500).
    App();

    // Sets up view `view`, its camera and viewport: 0 the play area, 2
    // the whole screen. Inline; the copy is the stage's object's
    // (0x401390).
    void SetView(int view)
    {
        this->camera = &this->cameras[view];
        this->SetupCamera3D(this->camera);
        this->d3dDevice->SetViewport(&this->camera->viewport);
        this->view = view;
    }
    // SetView, setting the camera up through SetupCamera. Inline; the
    // copy is at 0x403d90.
    void SetView2D(int view)
    {
        this->camera = &this->cameras[view];
        this->SetupCamera(this->camera);
        this->d3dDevice->SetViewport(&this->camera->viewport);
        this->view = view;
    }
    // Sets up the camera and the projection of `camera` (0x430590).
    void SetupCamera(Camera *camera);
    // Sets up the current camera and its viewport again. Inline; the copy
    // is at 0x4013e0.
    void ResetView()
    {
        this->SetupCamera(this->camera);
        this->d3dDevice->SetViewport(&this->camera->viewport);
    }
    // Sets up the projection of `camera` in 3D, from its eye, target
    // and up vectors (0x4306b0).
    void SetupCamera3D(Camera *camera);
    // Sets up the three cameras looking down from z 1000 with a field of
    // view of pi / 6: the whole screen (2) and the two sides' play areas
    // (0, 1) (0x4307c0).
    void InitCameras();
    // ResetView with SetupCamera3D. Inline; the copy is at 0x401410.
    void ResetView3D()
    {
        this->SetupCamera3D(this->camera);
        this->d3dDevice->SetViewport(&this->camera->viewport);
    }
    // Makes view `view`'s camera the current one. Inline; the copy is at
    // 0x401440.
    void SetCamera(int view)
    {
        this->camera = &this->cameras[view];
    }
    // The config's noFog option. Inline; the copy is at 0x401460.
    int GetNoFog()
    {
        return this->config.flags.noFog;
    }
    // Bit 3 of stateFlags, as th08's GetNoScore. Inline; the copy is at
    // 0x420180.
    int GetNoScore()
    {
        return this->stateFlags >> 3 & 1;
    }
    // The config's other options, as th08's getters, out of line
    // (0x42d2b0, 0x42d2c0, 0x42d2d0, 0x42ed30, 0x42ed40, 0x42ed50,
    // 0x42ed60).
    int GetForce16BitTextures()
    {
        return this->config.flags.force16BitTextures;
    }
    int GetReferenceRasterizer()
    {
        return this->config.flags.referenceRasterizer;
    }
    int GetWindowed()
    {
        return this->config.windowed;
    }
    int GetNoDirectInput()
    {
        return this->config.flags.noDirectInput;
    }
    int GetPreloadBgm();
    int GetNoVsync();
    int GetNoTextRenderDetect();
    // Saves the back buffer to the bitmap `path`, as th08's. Returns 1
    // for a back buffer format it does not know, else 0 (0x42f540).
    int SaveScreenshot(char *path);
    // Whether subThread is set, as th08's. Inline; the copy the others
    // call is at 0x42ed70.
    BOOL HasSubThread()
    {
        return this->subThread != NULL;
    }
    // Enters and leaves criticalSections[idx], counting the depth in
    // criticalSectionDepths, as th08's (0x42b130, 0x42b160).
    void EnterCriticalSection(int idx);
    void LeaveCriticalSection(int idx);
    // Puts a record of `value` with `unknown` and `seed` in side `side`'s
    // list, before the first greater one, and makes it the side's last
    // (0x42e9e0).
    void AddRecord(int side, int value, WORD unknown, WORD seed);
    // Puts a record of `value` with `seed` and the side's last record's
    // unk_0 in the side's list, before the first greater one, marked
    // with unk_8 1 (0x42eac0).
    void InsertRecord(int side, int value, WORD seed);
    // Takes the side's first record off its list: returns its unk_0,
    // or the side's input if its unk_8 is set, and stores its seed
    // and unk_8 (0x42eb80).
    WORD PopRecord(int side, WORD *seed, int *unk_8);
    // Whether the side has a record and both sides' first records have
    // the same value (0x42ecf0).
    BOOL RecordsMeet(int side);
    // Resets the run: clears the fields from 0x458 and the records,
    // restarts the timers, adds each side a record of -9999 with the
    // random seed, and sets network play's unk_a8 (0x42ec30).
    void ResetRun();
    // Looks `id` up in g_BgmInfos and loads its MIDI file into slot `id`,
    // or queues the wave file of the same name, as th08's LoadMidi.
    // Returns 0 for MIDI, else 1 (0x42fc20).
    int LoadBgm(int id);
    // Plays the music loaded last. Returns 0 (0x42fcf0).
    int PlayBgm();
    // Stops the music, as th08's StopMusic. Returns 0, or -1 without
    // music (0x42fd30).
    int StopMusic();
    // Looks `id` up in g_BgmInfos and frees what LoadBgm loaded. Returns
    // 0, 1 for an unknown id, or -1 without music (0x42fd90).
    int UnloadBgm(int id);
    // Fades the music out over `seconds`, as th08's FadeOutMusic; the
    // wave fade is stretched while the game runs slow. Returns 0, or -1
    // without music (0x42fe20).
    int FadeOutMusic(float seconds);
    // Plays the piece `id` of the table at 0x4a21d0; what `arg` does is
    // not known. Returns 1 for an unknown id (0x431930).
    int PlayMusicById(int id, int arg);
    // Plays the MIDI file `path`, or queues the wave file of the same
    // name, as th08's PlayMusic, and marks piece `heard` heard. Returns
    // 0, or -1 without music (0x431a30).
    int PlayMusic(char *path, int heard);
    // Advances `frames` by one frame, or by gameSpeed in `subFrame`'s
    // fractions while the game runs slow (0x42f4f0).
    void TickTimer(int *frames, float *subFrame);
    // Starts the three loading sprites at `pos` unless they show, as
    // th08's (0x4302b0).
    void StartLoadingSprites(D3DXVECTOR3 *pos);
    // Interrupts the loading sprites with 1 if they show, and finishes
    // g_LoadingScreenEffect, as th08's (0x430360).
    void StopLoadingSprites();
    // StartLoadingSprites, then has the screen copied to surface 8, as
    // th08's (0x4303c0).
    void StartLoadingSpritesOverScreen(D3DXVECTOR3 *pos);
    // Like StopLoadingSprites, with interrupt 2 and loadingState 2, as
    // th08's (0x4304a0).
    void FinishLoadingSprites();
    // Starts g_LoadingScreenEffect, a ScreenEffect of type `type` + 5,
    // unless one runs, as th08's (0x430500).
    void StartLoadingScreenEffect(int type);
    // Initializes and deletes criticalSections (0x430530, 0x430560).
    void InitCriticalSections();
    void DeleteCriticalSections();
    // Flushes the sprites and turns fog on unless fogState is 1 already,
    // as th08's; returns SetRenderState's result, or 0 (0x42fec0).
    int EnableFog();
    // The same, turning fog off if fogState is set (0x42ff00).
    int DisableFog();
    // Flushes the sprites and sets render state `state` to `value`
    // (0x42ff40).
    int SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
    // Adds the time since the last call to g_RunTime, as th08's
    // (0x42ff70).
    void AddPlayTime();
    // The same for g_PlayTime, restarting from 0 when the clock wrapped
    // (0x430060).
    void AddTotalPlayTime();
    // Returns 0 if verFile lists `version` with `size` and `checksum`,
    // or if `version` is "debug", and -1 otherwise, as th08's
    // (0x430150).
    int CheckVersionFile(char *version, DWORD size, DWORD checksum);
    // Reads the config from `path` into config, or sets the defaults if
    // it is missing or wrong, with the music mode from whether
    // thbgm.dat is there, as th08's; logs the options and writes the
    // config back. Returns 0, or -1 if thbgm.dat has the wrong version or
    // the config cannot be written (0x42f7c0).
    int LoadConfig(char *path);
    // Asks the sub thread to close (closeSubThread) and waits for it,
    // as th08's (0x430230).
    void CloseSubThread();
    // Runs `proc` with `arg` in the sub thread, as th08's (0x431ae0).
    int StartSubThread(LPTHREAD_START_ROUTINE proc, LPVOID arg);
    // The chain callbacks, as th08's: the update element's callback and
    // its added and deleted callbacks (0x431110, 0x432090, 0x431b70),
    // and the draw elements' at priorities 0x1f, 0 and 2 (0x431b30,
    // 0x431540, 0x42ed80).
    static int OnUpdate(App *app);
    static int AddedCallback(App *app);
    static int DeletedCallback(App *app);
    static int OnDraw(App *app);
    static int OnDrawLogo(App *app);
    static int OnDrawLoading(App *app);
    // Resets the scenes and adds the update and draw elements, as th08's.
    // Returns 0, or what adding the update element returned (0x432180).
    static int RegisterChain();
    // Loads the game's resources in the sub thread, as th08's (0x431d10).
    static void LoadingThread(App *app);
    // Runs a frame of network play; 1 to run another, 2 to skip the
    // frame, 3 when it ended and 4 when it failed (0x430aa0).
    int UpdateNetPlay();
    // Clears subThread, closeSubThread and subThreadRunning (0x41a997).
    void ResetSubThread();
    // Zeroes unk_77c to unk_790 (0x423382).
    void ResetClockDriftCounters();

    // WinMain's instance.
    HINSTANCE hInstance;
    // Created by CreateD3d.
    IDirect3D8 *d3d;
    IDirect3DDevice8 *d3dDevice;
    // DirectInput, the keyboard and up to two pads, set up by
    // InitDirectInput.
    IDirectInput8 *dinput;
    IDirectInputDevice8 *keyboard;
    union
    {
        struct
        {
            IDirectInputDevice8 *joystick;
            IDirectInputDevice8 *joystick2;
        };
        // The same, by player.
        IDirectInputDevice8 *joysticks[2];
    };
    DIDEVCAPS joystickCaps;
    DIDEVCAPS joystickCaps2;
    // The game's window, as th08's.
    HWND hwnd;
    // The views' cameras.
    Camera cameras[3];
    // The camera of the view SetView last set up.
    Camera *camera;
    // The view SetView last set up.
    int view;
    // The parameters the device is created with, as th08's.
    D3DPRESENT_PARAMETERS presentParams;
    // The 16 ms timer LoadingThread starts if stateFlags bit 6 is set.
    MidiTimer *timer;
    GameConfig config;
    // Counts the frames, as th08's.
    int frames;
    int unk_458;
    int unk_45c;
    int unk_460;
    // The time ResetRun ran.
    DWORD unk_464;
    DWORD unk_468;
    DWORD unk_46c;
    unsigned char pad_470[0x474 - 0x470];
    DWORD unk_474;
    int unk_478;
    // Each side's ten records, ordered by value, and its last one.
    RunRecord records[2][10];
    RunRecord lastRecords[2];
    // Drawn by OnDraw while unk_478 is set.
    Sprite *unk_584;
    // When the current connection attempt started.
    DWORD connectStartTime;
    // The scene running, as th08's.
    int curScene;
    // The scene to go to.
    int scene;
    int prevScene;
    // Set when the game ends, unless it goes on to scene 3, 10 or 12:
    // the game's objects then free their resources, as th08's.
    int freesResources;
    // Set when the game restarts, as th08's isRestarting.
    int unk_59c;
    unsigned char pad_5a0[0x5a4 - 0x5a0];
    // How many more frames to redraw, as th08's: set to 2 when the
    // device is lost, cleared by LoadingThread.
    int redrawFrames;
    // Set while the frame rate is not shown, as th08's.
    int hideFps;
    // Set when the config turns vertical sync off.
    int noVsync;
    // Set while the full screen asks for 60 Hz, as th08's.
    int wants60Hz;
    // Cleared by InitD3dDevice, as th08's clearedOnD3dInit.
    int clearedOnD3dInit;
    // Below 1 when the game runs slow.
    float gameSpeed;
    MidiPlayer *midi;
    // DrawFps's tally of the frames drawn against the frames due, as
    // th08's, while a game plays.
    float achievedFrames;
    float expectedFrames;
    // The frame rate shown, as th08's; a replay puts the recorded one
    // back.
    short roundedFps;
    unsigned char pad_5ca[0x5cc - 0x5ca];
    // The ANM file of the menus' text, as th08's.
    AnmLoaded *textAnm;
    // The ANM file of the loading sprites.
    AnmLoaded *loadingAnm;
    union
    {
        DWORD stateFlags;
        struct
        {
            unsigned int unusedBits5d4_0 : 9;
            // Whether the scene's objects load their resources, as th08's.
            unsigned int loadsResources : 1;
        };
    };
    // The device's capabilities, as th08's.
    D3DCAPS8 caps;
    // The loading thread, as th08's.
    HANDLE subThread;
    DWORD subThreadId;
    // Set to ask the sub thread to end.
    int closeSubThread;
    int subThreadRunning;
    // Set while LoadingThread runs, as th08's.
    int loadingThreadState;
    CRITICAL_SECTION criticalSections[5];
    // How deep each of criticalSections is entered.
    BYTE criticalSectionDepths[5];
    unsigned char pad_73d[0x740 - 0x73d];
    // 0 while the loading sprites are hidden, 1 while they show, 2 once
    // they are finishing.
    int loadingState;
    // The performance counter's frequency, or 0 if there is none, as
    // th08's App::perfFrequency.
    DWORD perfFrequency[2];
    unsigned char pad_74c[0x77c - 0x74c];
    int unk_77c;
    // Set when the frame rate is low, as th08's.
    int fpsWarning;
    int unk_784;
    int unk_788;
    int unk_78c;
    int unk_790;
    // Whether fog is on, as EnableFog and DisableFog last set it.
    DWORD fogState;
    // timeGetTime() when AddPlayTime and AddTotalPlayTime last ran.
    DWORD lastPlayTimeTick;
    DWORD lastTotalPlayTimeTick;
    // The executable's checksum and size, for CheckVersionFile, as
    // th08's.
    DWORD exeChecksum;
    DWORD exeSize;
    // The contents of the version file, as th08's: lines of a version,
    // a size and a checksum.
    DWORD verFileSize;
    char *verFile;
};
extern App g_App;

// Whether the game's objects free their resources (0x415cf0).
int FreesResources();

// The loading sprites, at 0x4b38b0.
extern Sprite g_LoadingScreenSprites[3];

// DirectInput enumeration callbacks: creates the first two pads, as
// th08's EnumJoysticksCallback (0x42edd0), and sets the range of the
// first's and the second's axes to -1000..1000 (0x42ee30, 0x42ee90).
BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE *pdidInstance, VOID *pContext);
BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *pContext);
BOOL CALLBACK EnumAxesCallback2(const DIDEVICEOBJECTINSTANCE *pdidoi, VOID *pContext);

// Loads score.dat (0x421ea0), and writes it back through scoretmp
// (0x421f10).
int LoadScoreFile();
int SaveScoreDat();

class ScreenEffect;
// The loading screen's fade, at 0x4b30f8.
extern ScreenEffect *g_LoadingScreenEffect;
