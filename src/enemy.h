// The enemies, one manager per side ("EnemyCtrlInf").

#pragma once

#include "chain.h"
#include "timer.h"

// The head of an ECL file, as th08's: then the offsets of the timelines.
struct EclHeader
{
    int version;
    short subCount;
    short timelineCount;
    void *timelines[1];
};

// An ECL file and the state of its scripts (0x188 bytes). Only the
// fields known so far.
struct EclManager
{
    // Loads the ECL file `path`. Returns 0, or -1 on failure (0x405660).
    int Load(char *path);
    // Frees the ECL file (0x406830).
    void Unload();

    // How many timelines the file has. Inline, as th08's; the copy is
    // at 0x40f160.
    int GetTimelineCount()
    {
        return this->file->timelineCount;
    }
    // Timeline `i` of the file. Inline, as th08's; the copy is at
    // 0x40f170.
    void *GetTimeline(int i)
    {
        return this->file->timelines[i];
    }

    EclHeader *file;
    unsigned char pad_4[0x188 - 0x4];
};
ASSERT_SIZE(EclManager, 0x188);

// An enemy (0x5430 bytes). Only the fields known so far.
struct Enemy
{
    // Frees the blocks of unknown33d8 (0x4100e0).
    void FreeUnknown33d8();

    unsigned char pad_0[0x337c];
    DWORD unknown337c;
    unsigned char pad_3380[0x33d8 - 0x3380];
    void *unknown33d8[4];
    unsigned char pad_33e8[0x5430 - 0x33e8];
};
ASSERT_SIZE(Enemy, 0x5430);

// A side's enemy manager (0x2ac450 bytes). Only the fields known so far.
class EnemyManager
{
public:
    // Frees the enemies' resources, removes the manager's chain elements
    // and deletes it, if any (0x411f80).
    static void RemoveChain(EnemyManager *mgr);
    // Resets the enemies for a new round (0x411fd0).
    void Reset();

    // enemy.ecl, and the character's plXX.ecl.
    EclManager ecl;
    EclManager playerEcl;
    ChainElem *calcElem;
    ChainElem *drawElem;
    ChainElem *drawElem2;
    // Only while it is 0 does Reset roll g_TimelineOrder; the side,
    // perhaps.
    int unknown31c;
    unsigned char pad_320[0x5758 - 0x320];
    Enemy enemies[0x80];
    unsigned char pad_2a6f58[0x2ac3d4 - 0x2a6f58];
    unsigned char unknown2ac3d4;
    unsigned char pad_2ac3d5[0x2ac3e0 - 0x2ac3d5];
    Timer unknown2ac3e0;
    Timer unknown2ac3ec;
    unsigned char pad_2ac3f8[0x2ac400 - 0x2ac3f8];
    // The timeline running.
    void *timeline;
    unsigned char pad_2ac404[0x2ac450 - 0x2ac404];
};
ASSERT_SIZE(EnemyManager, 0x2ac450);

// The order of the timelines, shared by the sides: an index in the low
// seven bits, and a random bit 7 (0x4a7c88).
extern unsigned char g_TimelineOrder[0x100];
