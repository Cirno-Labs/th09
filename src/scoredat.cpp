// The score file's functions, from 0x421aa0, as th08's scoredat.cpp.

#include "scoredat.h"
#include "unsorted.h"

int LoadLsnmChunk(ScoreDat *dat)
{
    Lsnm *chunk;
    int left;

    chunk = (Lsnm *)((BYTE *)dat + dat->dataOffset);
    memset(&g_Lsnm, 0, sizeof(g_Lsnm));
    g_Lsnm.magic = 0x4d4e534c;
    g_Lsnm.size = sizeof(Lsnm);
    g_Lsnm.size2 = sizeof(Lsnm);
    g_Lsnm.version = 1;
    g_Lsnm.unknown9 = 0;
    strcpy(g_Lsnm.name, "        ");
    left = dat->fileSize - dat->dataOffset;
    while (left > 0)
    {
        if (chunk->magic == 0x4d4e534c && chunk->version == 1)
        {
            g_Lsnm = *chunk;
            return 1;
        }
        left -= chunk->size;
        chunk = (Lsnm *)((BYTE *)chunk + chunk->size);
    }
    return 0;
}

int LoadPlstChunk(ScoreDat *dat)
{
    PlayRecord *chunk;
    int left;

    memset(&g_PlayRecord, 0, sizeof(g_PlayRecord));
    g_PlayRecord.size = sizeof(PlayRecord);
    g_PlayRecord.size2 = sizeof(PlayRecord);
    g_PlayRecord.bgmHeard[0] = 1;
    g_PlayRecord.openEntries[0] = 1;
    g_PlayRecord.openEntries[1] = 1;
    g_PlayRecord.openEntries[2] = 1;
    g_PlayRecord.openEntries[3] = 1;
    g_PlayRecord.openEntries[4] = 1;
    g_PlayRecord.openEntries2[0] = 1;
    g_PlayRecord.openEntries2[1] = 1;
    g_PlayRecord.openEntries2[2] = 1;
    g_PlayRecord.openEntries2[3] = 1;
    g_PlayRecord.openEntries2[4] = 1;
    g_PlayRecord.magic = 0x54534c50;
    g_PlayRecord.version = 3;
    g_PlayRecord.unknown9 = 0;
    chunk = (PlayRecord *)((BYTE *)dat + dat->dataOffset);
    left = dat->fileSize - dat->dataOffset;
    while (left > 0)
    {
        if (chunk->magic == 0x54534c50 && chunk->version == 3)
        {
            g_PlayRecord = *chunk;
        }
        left -= chunk->size;
        chunk = (PlayRecord *)((BYTE *)chunk + chunk->size);
    }
    return 0;
}

void ReleaseScoreDat(ScoreDat *dat)
{
    g_MemoryManager.Free(dat);
}

int LoadScore()
{
    ScoreDat *dat;

    dat = LoadScoreDat("score.dat");
    g_ScoreDatHeader = *dat;
    LoadHscrChunks(dat);
    LoadLsnmChunk(dat);
    LoadPlstChunk(dat);
    ReleaseScoreDat(dat);
    return 0;
}
