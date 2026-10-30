// The title menu. Only the fields known so far.

#pragma once

#include "common.h"
#include "sprite.h"
#include "unsorted.h"

// The network play's state ("NetworkInf", 0xd4 bytes), at 0x4b42d0.
// Only what is known so far.
struct Network
{
    unsigned char pad_0[0xa8];
    // While set, the menus do not skip the entries that are not open.
    int unknownA8;
};
extern Network *g_Network;

class MainMenu
{
public:
    // Queues sound effect `idx` with `arg` and plays it at once
    // (0x422f17).
    void PlaySoundNow(int idx, int arg);
    // Moves to state `state`, remembering the one it leaves in
    // prevState, and resets the frame counters, as th08's (0x422f39).
    void SetState(int state);
    // Gives every button of `mapping` assigned `button` the value
    // `replacement`, as th08's. The last argument is unused (0x422f67).
    void ReassignButton(ControllerMapping *mapping, short button, short replacement, int unused);
    // Shows `value` as two digits on `vm` and the sprite after it, or
    // hides both when it is negative, as th08's. Returns 0 (0x422fd1).
    int ShowButtonDigits(Sprite *vm, short value);
    // Moves cursors[idx] by `delta` among `count` entries, wrapping,
    // until it is on one that g_PlayRecord's openEntries marks open; entries[i] is
    // the entry at i. Not while unknownA8 of g_Network is set
    // (0x42304d; 0x4230a6 and 0x4230ff with openEntries2 and 3).
    void SkipClosedEntries(int idx, int delta, char *entries, int count);
    void SkipClosedEntries2(int idx, int delta, char *entries, int count);
    void SkipClosedEntries3(int idx, int delta, char *entries, int count);
    // Shows `left` and `right` in halves on the five sprites from 142
    // and the five from 148: 0xc3 for a full one, 0xc4 for a half, 0xc5
    // for an empty one. Not called (0x423182).
    void ShowHalves(int left, int right);
    // What OnDraw draws on top in states 11, 15, 13 and 14, and 12
    // (0x4234f6, 0x4239f6, 0x423d16, 0x4244a9).
    int DrawState11();
    int DrawState15();
    int DrawState13();
    int DrawState12();
    // States 3, 5 and 7 draw nothing more, but OnDraw's switch has
    // cases for them: they call something that compiles to nothing.
    int DrawNothing()
    {
        return 1;
    }

    // Moves cursor by up and down among `count` entries, wrapping, as
    // th08's MoveMenuCursor; returns -1, 1, or 0 when it did not move
    // (0x42457a).
    int MoveCursor(int count);
    // The same with left and right (0x424601).
    int MoveCursorLR(int count);
    // The same with all four directions, left and right first
    // (0x424671).
    int MoveCursor4(int count);
    // MoveCursorLR, up and down as left and right, on cursors[idx] with
    // player `idx`'s input (0x42474a).
    int MovePlayerCursor(int idx, int count);
    // The same on cursors[idx] with player `player`'s input (0x4247f1).
    int MovePlayerCursor2(int player, int idx, int count);
    // Draws the menu's sprites, then the state's own, as th08's
    // (0x424898).
    static int OnDraw(MainMenu *menu);

    int cursor;
    unsigned char pad_4[0x10 - 0x4];
    int cursors[6];
    int subState;
    // Counts the frames in the current state.
    int stateFrames;
    unsigned char pad_30[0x84 - 0x30];
    // The state before the current one.
    int prevState;
    unsigned char pad_88[0xc910 - 0x88];
    int unknownC910;
    unsigned char pad_c914[0x11b74 - 0xc914];
    int unknown11b74;
    unsigned char pad_11b78[0x11b90 - 0x11b78];
    // The title's ANM file.
    AnmLoaded *anm;
    unsigned char pad_11b94[0x11b98 - 0x11b94];
    // spriteCount sprites, drawn by OnDraw.
    Sprite *sprites;
    // Drawn after them when set.
    Sprite *helpSprite;
    unsigned char pad_11ba0[0x19d04 - 0x11ba0];
    Sprite unknown19d04[8];
    int spriteCount;
    int state;
    unsigned char pad_1b22c[0x1b230 - 0x1b22c];
    int unknown1b230;
    unsigned char pad_1b234[0x1b238 - 0x1b234];
    // While nonzero, OnDraw draws nothing.
    int loadingState;
};
