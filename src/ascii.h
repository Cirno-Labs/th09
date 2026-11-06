// The text drawn with ascii.anm, as th08's AsciiText.

#pragma once

#include <d3d8.h>
#include <d3dx8math.h>

#include "chain.h"
#include "common.h"
#include "sprite.h"

// A string queued by AsciiText::AddString, as th08's.
struct AsciiString
{
    char text[0x40];
    D3DXVECTOR3 pos;
    D3DCOLOR color;
    float scaleY;
    float scaleX;
    // Drawn with the second glyph set, in white; always 0 in th09.
    int highlight;
    // Drawn inside the play area's viewport.
    int inArena;
};
ASSERT_SIZE(AsciiString, 0x60);

// A number shown at a position for a while, as th08's.
struct AsciiPopup
{
    // Least significant first, or the glyphs to draw.
    BYTE digits[0xc];
    D3DXVECTOR3 pos;
    D3DCOLOR color;
    Timer timer;
    float scaleY;
    float scaleX;
    BYTE active;
    BYTE digitCount;
    unsigned char pad_32[0x6];
};
ASSERT_SIZE(AsciiPopup, 0x38);

// The pause menu, AsciiText's at 0x82a0, as th08's. Only what is known
// so far.
class PauseMenu
{
public:
    // Updates it while paused (0x434740), as th08's: a resume/quit menu
    // (states 1 and 2) whose quit asks for a confirmation (4-7); quitting
    // leaves for scene 10, or scene 11 in practice or on difficulty 4.
    // Pause (8) resumes and 0x200 goes to scene 1. Returns 0.
    int UpdatePauseMenu();
    // Draws it while paused (0x434e30).
    void DrawPauseMenu();

    int state;
    // Frames since the state changed.
    int stateFrames;
    Sprite sprites[7];
    // The copy of the play area behind the menu.
    Sprite background;
};
ASSERT_SIZE(PauseMenu, 0x1528);

// The continue menu, AsciiText's at 0x97c8, as th08's.
class ContinueMenu
{
public:
    // Updates it; nonzero stops AsciiText::OnUpdate (0x434e90). In a
    // replay, or once side 0's stats->unknown14 reaches 3, it leaves for
    // scene 1 or 6. Otherwise it asks whether to continue: no goes to
    // scene 6; yes counts unknown14 up, clears both sides' scores,
    // restarts the music and the round and gives side 0
    // config->unknownAc + 2 lives (mode 1 goes to scene 13 instead).
    int UpdateContinueMenu();
    // Draws it while in the continue menu (0x435370).
    void DrawContinueMenu();

    int state;
    int stateFrames;
    Sprite sprites[5];
    Sprite background;
};
ASSERT_SIZE(ContinueMenu, 0xfe0);

// A third menu, AsciiText's at 0xa7a8, up while
// g_GameManager.unknown13e is set. Only what is known so far.
class UnknownMenu
{
public:
    // Updates it; nonzero stops AsciiText::OnUpdate (0x4355f0). A menu
    // of three entries (sprites with scripts 0xd-0xf) that leaves for
    // scene 10, 1 or 6 by the one chosen; a replay leaves for scene 1.
    int UpdateUnknownMenu();
    // Draws it while it is up (0x435a90).
    void DrawUnknownMenu();

    int state;
    int stateFrames;
    Sprite sprites[3];
};
ASSERT_SIZE(UnknownMenu, 0x7f4);

// The ASCII text (0xe0ac bytes), at 0x4ce458. Only what is known so far.
class AsciiText
{
public:
    // Constructs the sprites, strings, menus and popups (0x435dc0).
    AsciiText();

    // The update element's deleted callback: frees ascii.anm and
    // capture.anm. Returns 0 (0x434260).
    static int DeletedCallback(AsciiText *text);
    // Removes the update and draw elements (0x434280).
    static void RemoveChain();
    // Queues `text` at `pos` with the current color, scale and arena
    // flag, as th08's (0x4342a0).
    void AddString(D3DXVECTOR3 *pos, char *text);
    // AddString with printf formatting (0x434330).
    void AddFormat(D3DXVECTOR3 *pos, char *fmt, ...);
    // Draws side `side`'s popups (0x4353f0).
    void DrawPopups(int side);
    // The update chain callback: runs the menus, and the popups while no
    // menu is up. Returns 3 when a menu asks to, else 1 (0x435b00).
    static int OnUpdate(AsciiText *text);
    // The draw chain callbacks: the strings, menus and demo icon, and
    // the popups of either side (0x435c00, 0x435c60).
    static int OnDraw(AsciiText *text);
    static int OnDraw2(AsciiText *text);
    // Adds the update element (priority 1) and the draw elements (0x22
    // and 0x17). Returns 0, or -1 if adding fails (0x435ca0).
    static int RegisterChain();
    // Zeroes the sprites, strings and popups and sets them up again, as
    // th08's (0x4343e0).
    void Initialize();
    // Called by the title after Initialize, where th08 starts the
    // sprites' scripts; empty here, and the linker folded it with
    // DebugPrint (0x43d2b0).
    void StartScripts();
    // The update element's added callback: loads ascii.anm (slot 1) and
    // capture.anm (slot 3) and initializes. Returns 0, or -1 if loading
    // fails (0x4344e0).
    static int AddedCallback(AsciiText *text);
    // Draws the queued strings, a glyph at a time (0x434540).
    void DrawStrings();
    // Shows `value` at `pos` in `color` for a while, among side
    // `side`'s popups, as th08's (0x4346a0).
    void AddPopup(int side, D3DXVECTOR3 *pos, int value, D3DCOLOR color);
    // Sets the pending interrupt of bossMarkers[`i`]. Called by the ECL
    // interpreter; the copy is at 0x4067d0.
    void SetBossMarkerInterrupt(int i, short interrupt)
    {
        this->bossMarkers[i].SetPendingInterrupt(interrupt);
    }
    // Sets the position of bossMarkers[`i`] (0x4067f0).
    void SetBossMarkerPos(int i, D3DXVECTOR3 *pos)
    {
        this->bossMarkers[i].pos = *pos;
    }
    // Sets bossMarkerStates[`i`] (0x40f890).
    void SetBossMarkerState(int i, int value)
    {
        this->bossMarkerStates[i] = value;
    }

    // Empties the queue of strings. Inline, as th08's; the copy is at
    // 0x430220.
    void ClearStrings()
    {
        this->stringCount = 0;
    }
    // Sets the alpha of the color text is queued with. Inline, as th08's;
    // the copy is at 0x4302a0.
    void SetAlpha(int alpha)
    {
        this->alpha = alpha;
    }
    // Sets the scale text is queued with. Inline in th08; this is the
    // copy at 0x4234dd, in an /O1 object.
    void SetScale(float x, float y);
    // Inline, as th08's; the copy is at 0x4343c0.
    void SetGlyphWidth(int width)
    {
        this->glyphWidth = width;
    }

    // The glyph DrawStrings draws each character with, and the digit the
    // popups are drawn with, and a second one, as th08's.
    Sprite glyph;
    Sprite digit;
    Sprite digit2;
    // As th08's summaryPopup to summaryPopup4, summaryDigit,
    // summaryBombIcon and bossMarkers.
    Sprite unknown7ec;
    Sprite unknownA90;
    Sprite unknownD34;
    Sprite unknownFd8;
    Sprite unknown127c;
    Sprite unknown1520;
    Sprite bossMarkers[4];
    int bossMarkerStates[4];
    AsciiString strings[0x100];
    // How many strings are queued.
    int stringCount;
    // The color, scale and arena flag text is queued with.
    union
    {
        D3DCOLOR color;
        struct
        {
            BYTE colorRgb[3];
            BYTE alpha;
        };
    };
    float scaleY;
    float scaleX;
    int inArena;
    int highlight;
    unsigned char pad_827c[0x8280 - 0x827c];
    // The advance of a character, in pixels.
    int glyphWidth;
    // The frames OnUpdate has run, as th08's.
    int frameCounter;
    // ascii.anm and capture.anm.
    AnmLoaded *anm;
    AnmLoaded *anm2;
    // Where the next popups go in popups[0] and popups[1].
    int nextPopup;
    int nextPopup2;
    unsigned char pad_8298[0x829c - 0x8298];
    int unknown829c;
    PauseMenu pauseMenu;
    ContinueMenu continueMenu;
    UnknownMenu unknownMenu;
    // Drawn while its script runs, as th08's.
    Sprite demoIcon;
    AsciiPopup popups[2][100];
    unsigned char pad_de00[0xde08 - 0xde00];
    Sprite unknownDe08;
};
ASSERT_SIZE(AsciiText, 0xe0ac);
extern AsciiText g_AsciiText;
// The update element and the draw elements (0x4dc504, 0x4ce438,
// 0x4dc524).
extern ChainElem g_AsciiCalcChain;
extern ChainElem g_AsciiTextElemDraw;
extern ChainElem g_AsciiDrawChain2;
