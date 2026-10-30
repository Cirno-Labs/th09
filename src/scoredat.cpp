// The score file's functions, from 0x421aa0, as th08's scoredat.cpp.

#include "scoredat.h"
#include "lzss.h"
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

ScoreDat *LoadScoreDat(char *path)
{
    BOOL found;
    Th9k *th9k;
    int left;
    ScoreChunk *chunk;
    int remaining;
    BYTE *p;
    WORD checksum;
    BYTE key;
    int n;
    BYTE *decoded;
    ScoreDat *dat;
    int readSize;
    ScoreDat *unpacked;

    LogPrintf(&g_Log, "info : score load\r\n");
    dat = (ScoreDat *)OpenPath(path, &readSize, 1);
    if (dat == NULL)
    {
    fail:
        LogPrintf(&g_Log, "info : score recreate\r\n");
        if (dat != NULL)
        {
            g_MemoryManager.Free(dat);
        }
        dat = (ScoreDat *)g_MemoryManager.Alloc(sizeof(ScoreDat), "scorefile");
        dat->dataOffset = sizeof(ScoreDat);
        dat->fileSize = sizeof(ScoreDat);
        dat->version = 4;
        goto done;
    }
    if (readSize < sizeof(ScoreDat))
    {
        LogPrintf(&g_Log, "warning : score.dat size is short\r\n");
        g_MemoryManager.Free(dat);
        goto fail;
    }
    decoded = DecryptData((BYTE *)dat, readSize, 0x3a, 0xcd, 0x100, 0xc00);
    g_MemoryManager.Free(dat);
    dat = (ScoreDat *)decoded;
    remaining = readSize - 2;
    checksum = 0;
    key = 0;
    n = 0;
    p = (BYTE *)dat + 1;
    while (remaining > 0)
    {
        key += *p;
        key = ((key & 0xe0) >> 5) | ((key & 0x1f) << 3);
        p[1] ^= key;
        if (n >= 2)
        {
            checksum += p[1];
        }
        p++;
        remaining--;
        n++;
    }
    if (dat->checksum != checksum)
    {
        LogPrintf(&g_Log, "warning : score.dat chksum error\r\n");
        goto fail;
    }
    if (dat->dataOffset != sizeof(ScoreDat))
    {
        LogPrintf(&g_Log, "warning : header size is mismatch\r\n");
        goto fail;
    }
    if (dat->version != 4)
    {
        LogPrintf(&g_Log, "warning : score.dat version mismatch\r\n");
        goto fail;
    }
    unpacked = (ScoreDat *)g_MemoryManager.Alloc(sizeof(ScoreDat) + 0xa0000, "scorefile2");
    *unpacked = *dat;
    LzssDecompress((BYTE *)dat + sizeof(ScoreDat), dat->packedSize, (BYTE *)unpacked + sizeof(ScoreDat),
                   dat->dataSize);
    g_MemoryManager.Free(dat);
    dat = unpacked;
    left = dat->fileSize;
    found = FALSE;
    chunk = (ScoreChunk *)((BYTE *)dat + dat->dataOffset);
    left -= dat->dataOffset;
    while (left > 0)
    {
        if (chunk->magic == 0x4b394854)
        {
            found = TRUE;
            th9k = (Th9k *)chunk;
        }
        if (chunk->size == 0)
        {
            LogPrintf(&g_Log, "warning : score.dat chapter size is ZERO\r\n");
            goto fail;
        }
        left -= chunk->size;
        chunk = (ScoreChunk *)((BYTE *)chunk + chunk->size);
    }
    if (!found || th9k->version != 1)
    {
        LogPrintf(&g_Log, "warning : score.dat version mismatch\r\n");
        goto fail;
    }
done:
    return dat;
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
