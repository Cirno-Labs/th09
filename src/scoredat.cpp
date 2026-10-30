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
