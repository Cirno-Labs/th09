// The score file, score.dat, as th08's.

#pragma once

#include <windows.h>

#include "common.h"

// The score file's header, as th08's, with the sizes in place of the
// score list.
struct ScoreDat
{
    BYTE unusedByte00;
    // Where the key starts; random.
    BYTE key;
    // The sum of the bytes from the fifth on.
    WORD checksum;
    WORD version;
    BYTE secondaryKey;
    BYTE unusedByte07;
    // Where the chunks start.
    int dataOffset;
    // The size of the whole file, unpacked.
    int fileSize;
    // The size of the chunks, unpacked and packed.
    int dataSize;
    int packedSize;
};
ASSERT_SIZE(ScoreDat, 0x18);
// The header of the score file last loaded (0x4ac840).
extern ScoreDat g_ScoreDatHeader;

// "LSNM": the last name entered (version 1).
struct Lsnm
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unknown9;
    unsigned char pad_a[0x2];
    char name[0xc];
};
ASSERT_SIZE(Lsnm, 0x18);
// At 0x4a8168.
extern Lsnm g_Lsnm;

// Reads the score file at `path`, decrypted and unpacked, as th08's
// LoadScoreDat (0x421c80).
ScoreDat *LoadScoreDat(char *path);
// Reads the high scores from `dat`'s "HSCR" chunks (0x4218e0).
int LoadHscrChunks(ScoreDat *dat);
// Sets g_Lsnm to blanks, then reads it from `dat`'s chunk; returns
// whether there is one (0x421aa0).
int LoadLsnmChunk(ScoreDat *dat);
// Resets g_PlayRecord, then reads it from `dat`'s "PLST" chunks
// (0x421b80).
int LoadPlstChunk(ScoreDat *dat);
// Frees `dat` (0x421e90).
void ReleaseScoreDat(ScoreDat *dat);
