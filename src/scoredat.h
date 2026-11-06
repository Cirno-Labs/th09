// The score file, score.dat, as th08's.

#pragma once

#include <windows.h>

#include "common.h"

// The score file's header, as th08's, with the sizes in place of the
// score list.
struct ScoreDat
{
    ScoreDat();

    BYTE pad_00;
    // Where the key starts; random.
    BYTE key;
    // The sum of the bytes from the fifth on.
    WORD checksum;
    WORD version;
    // Random; never read.
    BYTE randomByte;
    BYTE pad_07;
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
    BYTE unk_9;
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
    BYTE unk_9;
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

// A time kept in the play statistics, as th08's plst fields.
struct PlayTime
{
    DWORD hours;
    DWORD minutes;
    DWORD seconds;
    DWORD milliseconds;
};

// The record of what has been played, at 0x4a8180, around g_RunTime
// and g_PlayTime (+0xc, +0x1c). Only what is known so far.
struct PlayRecord
{
    // Counts a clear of character `character` on difficulty
    // `difficulty`, up to 999999 (0x40e3a0).
    void AddClear(char character, int difficulty)
    {
        int *clears = &this->clearFlags[character][difficulty];
        if (*clears < 999999)
        {
            *clears += 1;
        }
    }
    // Opens entry `entry` of the three menus' lists (0x40e3d0, 0x40e3e0,
    // 0x40e3f0).
    void OpenEntry(char entry)
    {
        this->openEntries[entry] = 1;
    }
    void OpenEntry2(char entry)
    {
        this->openEntries2[entry] = 1;
    }
    void OpenEntry3(char entry)
    {
        this->openEntries3[entry] = 1;
    }
    // Whether character `character` has a clear on any of the five
    // difficulties (0x40e400).
    BOOL HasCleared(char character)
    {
        if (this->clearFlags[character][0] != 0 || this->clearFlags[character][1] != 0 ||
            this->clearFlags[character][2] != 0 || this->clearFlags[character][3] != 0 ||
            this->clearFlags[character][4] != 0)
        {
            return TRUE;
        }
        return FALSE;
    }
    // Character `character`'s count for difficulty 5 (0x40e450).
    int GetClears5(char character)
    {
        return this->clearFlags[character][5];
    }
    // Counts one more for character `character`'s difficulty 5, up to
    // 999999 (0x4158e0).
    void AddClear5(char character)
    {
        int *clears = &this->clearFlags[character][5];
        if (*clears < 999999)
        {
            *clears += 1;
        }
    }
    // Character `character`'s count for difficulty `difficulty`, or,
    // when it is negative, those of the first four or'ed together
    // (0x4234a7).
    int GetClearFlags(char character, int difficulty);

    // The score file's "PLST" chunk header: version 3.
    DWORD magic;
    WORD size;
    WORD size2;
    BYTE version;
    BYTE unk_9;
    unsigned char pad_a[0xc - 0xa];
    PlayTime runTime;
    PlayTime playTime;
    // Which pieces of music have been heard, by BgmInfo::unk_0c.
    BYTE bgmHeard[0x20];
    // Which entries of three of the menus' lists are open.
    BYTE openEntries[16];
    BYTE openEntries2[16];
    BYTE openEntries3[16];
    // Each character's clears on each difficulty, to the end of the
    // record.
    int clearFlags[16][6];
};
ASSERT_SIZE(PlayRecord, 0x1fc);
extern PlayRecord g_PlayRecord;

// How long the game has run (0x4a818c), and played (0x4a819c).
#ifdef TH09_EXE
#define g_RunTime (g_PlayRecord.runTime)
#define g_PlayTime (g_PlayRecord.playTime)
#else
extern PlayTime g_RunTime;
extern PlayTime g_PlayTime;
#endif
