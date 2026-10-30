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

// Which entries of three of the menus are open, 16 each (0x4a81cc,
// 0x4a81dc, 0x4a81ec).
extern BYTE g_OpenEntries[16];
extern BYTE g_OpenEntries2[16];
extern BYTE g_OpenEntries3[16];

// The record of what has been played, at 0x4a8180, around g_RunTime
// and g_PlayTime (+0xc, +0x1c) and g_OpenEntries (+0x4c). Only what is
// known so far.
struct PlayRecord
{
    // Character `character`'s flags for difficulty `difficulty`, or,
    // when it is negative, those of the first four together
    // (0x4234a7).
    int GetClearFlags(char character, int difficulty);

    unsigned char pad_0[0x7c];
    int clearFlags[16][6];
};
extern PlayRecord g_PlayRecord;

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
    // until it is on one that g_OpenEntries marks open; entries[i] is
    // the entry at i. Not while unknownA8 of g_Network is set
    // (0x42304d; 0x4230a6 and 0x4230ff with g_OpenEntries2 and 3).
    void SkipClosedEntries(int idx, int delta, char *entries, int count);
    void SkipClosedEntries2(int idx, int delta, char *entries, int count);
    void SkipClosedEntries3(int idx, int delta, char *entries, int count);

    unsigned char pad_0[0x10];
    int cursors[6];
    int subState;
    // Counts the frames in the current state.
    int stateFrames;
    unsigned char pad_30[0x84 - 0x30];
    // The state before the current one.
    int prevState;
    unsigned char pad_88[0xc910 - 0x88];
    int unknownC910;
    unsigned char pad_c914[0x11b90 - 0xc914];
    // The title's ANM file.
    AnmLoaded *anm;
    unsigned char pad_11b94[0x1b228 - 0x11b94];
    int state;
    unsigned char pad_1b22c[0x1b230 - 0x1b22c];
    int unknown1b230;
};
