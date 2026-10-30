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
    float scaleX;
    float scaleY;
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
    float scaleX;
    float scaleY;
    BYTE active;
    BYTE digitCount;
    unsigned char pad_32[0x6];
};
ASSERT_SIZE(AsciiPopup, 0x38);

// The ASCII text (0xe0ac bytes), at 0x4ce458. Only what is known so far.
class AsciiText
{
public:
    // Removes the update and draw elements (0x434280).
    static void RemoveChain();
    // Queues `text` at `pos` with the current color, scale and arena
    // flag, as th08's (0x4342a0).
    void AddString(D3DXVECTOR3 *pos, char *text);
    // AddString with printf formatting (0x434330).
    void AddFormat(D3DXVECTOR3 *pos, char *fmt, ...);
    // Zeroes the sprites, strings and popups and sets them up again, as
    // th08's (0x4343e0).
    void Initialize();
    // The update element's added callback: loads ascii.anm (slot 1) and
    // capture.anm (slot 3) and initializes. Returns 0, or -1 if loading
    // fails (0x4344e0).
    static int AddedCallback(AsciiText *text);
    // Draws the queued strings, a glyph at a time (0x434540).
    void DrawStrings();
    // Shows `value` at `pos` in `color` for a while, among side
    // `side`'s popups, as th08's (0x4346a0).
    void AddPopup(int side, D3DXVECTOR3 *pos, int value, D3DCOLOR color);

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
    unsigned char pad_7ec[0x2264 - 0x7ec];
    AsciiString strings[0x100];
    // How many strings are queued.
    int stringCount;
    // The color, scale and arena flag text is queued with.
    D3DCOLOR color;
    float scaleX;
    float scaleY;
    int inArena;
    int highlight;
    unsigned char pad_827c[0x8280 - 0x827c];
    // The advance of a character, in pixels.
    int glyphWidth;
    unsigned char pad_8284[0x8288 - 0x8284];
    // ascii.anm and capture.anm.
    AnmLoaded *anm;
    AnmLoaded *anm2;
    // Where the next popups go in popups[0] and popups[1].
    int nextPopup;
    int nextPopup2;
    unsigned char pad_8298[0x829c - 0x8298];
    int unknown829c;
    // The pause and continue menus, as th08's: two ints and 8 and 6
    // sprites.
    unsigned char pauseMenu[0x1528];
    unsigned char continueMenu[0xfe0];
    unsigned char pad_a7a8[0xb240 - 0xa7a8];
    AsciiPopup popups[2][100];
    unsigned char pad_de00[0xe0ac - 0xde00];
};
ASSERT_SIZE(AsciiText, 0xe0ac);
extern AsciiText g_AsciiText;
// The update element and a draw element (0x4dc504, 0x4ce438).
extern ChainElem g_AsciiCalcChain;
extern ChainElem g_AsciiTextElemDraw;
