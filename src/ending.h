// The ending and staff roll.

#pragma once

#include <windows.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"
#include "timer.h"

// The ending ("EndingInf"), laid out as th08's: a text script of lines
// shown one at a time, with commands after '@'. Only the scroll, the
// sprites and fadeColor are checked so far.
class Ending
{
public:
    // Constructs the Sprites, then zeroes the whole ending and starts
    // its timers, as th08's inline one (0x40ef60).
    Ending();
    // Reads a number at the cursor and moves the cursor past it and the
    // NULs after it, as th08's (0x40e0d0).
    int ReadInt();
    // Draws the fade over the screen, if it shows. Where th08 stepped the
    // fade here, this only draws it (0x40e130).
    void DrawFade();
    // Steps the fade, as th08's DrawFade did before drawing it
    // (0x40e170).
    void UpdateFade();
    // The update chain element's deleted callback: frees what the
    // ending loaded, removes the draw chain element, frees the ending and
    // clears bit 0x400 of g_App.stateFlags, as th08's (0x40e2f0).
    static int DeletedCallback(Ending *ending);
    // The draw chain element's callback: draws the background, the
    // sprites and the fade, as th08's. Returns 1 (0x40e480).
    static int OnDraw(Ending *ending);
    // Loads the script at `path` and starts it. Returns 0, or -1 after
    // logging an error, as th08's (0x40e4f0).
    int Load(char *path);
    // The update chain element's added callback: loads the staff ANM,
    // records the clear and loads the ending's script (0x40e580).
    static int AddedCallback(Ending *ending);
    // Runs the script until it waits: shows text lines, runs the '@'
    // commands. Returns 0, or -1 at its end or on failure, as th08's
    // (0x40e8a0).
    int RunScript();
    // The update chain element's callback: runs the script and steps
    // the sprites, up to 9 times a frame while skipping, as th08's.
    // Returns 0 at the script's end, otherwise 1 (0x40efe0).
    static int OnUpdate(Ending *ending);
    // Creates the ending's chain elements: OnUpdate at priority 5 with
    // its added and deleted callbacks, OnDraw at 4, as th08's. Returns 0,
    // or -1 if adding fails (0x40f080).
    static int RegisterChain();

    ChainElem *calcChain;
    ChainElem *drawChain;
    // Where the background image (surface 0) is copied from.
    float scrollX;
    float scrollY;
    float scrollSpeed;
    // The text lines and the pictures.
    Sprite sprites[0x10];
    char *file;
    int canSkip;
    int nextCanSkip;
    AnmLoaded *anm;
    Timer timer;
    Timer lineTimer;
    Timer waitTimer;
    int waitMinFrames;
    int lineMinFrames;
    int lineDelay;
    int lineDelayFast;
    unsigned char pad_2a98[0x4];
    int line;
    D3DCOLOR textColor;
    // The fade over the screen, drawn while its alpha is not 0.
    D3DCOLOR fadeColor;
    int fadeTimer;
    int fadeDuration;
    int fadeType;
    // The script position.
    char *cursor;
};
ASSERT_SIZE(Ending, 0x2ab8);
