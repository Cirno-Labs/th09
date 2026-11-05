// The score file, score.dat, as th08's.

#pragma once

#include <windows.h>

#include "common.h"

// The score file's header, as th08's, with the sizes in place of the
// score list.
struct ScoreDat
{
    ScoreDat();

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

// The header every chunk of the score file starts with.
struct ScoreChunk
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unknown9;
    unsigned char pad_a[0x2];
};
ASSERT_SIZE(ScoreChunk, 0xc);

// "TH9K": the chunk with the score file's version (1).
struct Th9k
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    unsigned char pad_9[0x3];
};
ASSERT_SIZE(Th9k, 0xc);

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

// The version chunk ("VRSM") that ends score.dat, as th08's.
struct Vrsm
{
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE clearedOnSave;
    unsigned char pad_a[0x2];
    char gameVersion[0x8];
    DWORD exeSize;
    DWORD exeChecksum;
};
ASSERT_SIZE(Vrsm, 0x1c);

// Reads the score file at `path`, decrypted and unpacked, as th08's
// LoadScoreDat (0x421c80).
ScoreDat *LoadScoreDat(char *path);
// Puts `entry` in its high score table, moving the lower entries down.
// Returns its rank, or 99 if it is not high enough (0x421800).
int InsertHighScore(struct Hscr *entry);
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
