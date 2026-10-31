#include <string.h>

#include "replay.h"
#include "gamemanager.h"
#include "gui.h"
#include "lzss.h"
#include "unsorted.h"

void PlayerInput::UpdateRepeat2()
{
    WORD buttons = this->unknown2c;
    int i;
    WORD *held;

    this->unknown4 = 0;
    held = this->heldFrames2;
    for (i = 0; i < 16; i++, held++)
    {
        if (buttons & 1)
        {
            (*held)++;
            if (*held >= 26)
            {
                this->unknown30 |= 1;
                *held -= 8;
            }
        }
        else
        {
            *held = 0;
        }
        buttons >>= 1;
    }
    this->unknown32 = (this->unknown2c ^ this->unknown2e) & this->unknown2c;
    this->unknown34 = ~this->unknown2c & (this->unknown2c ^ this->unknown2e);
}

void Rng::SetSeed(WORD seed)
{
    this->seed = seed;
}

void Rng::ResetGenerated()
{
    this->generated = 0;
}

int ReplayManager::OnUpdateMode1b(ReplayManager *mgr)
{
    if ((g_GameManager.stateFlags & 4) && !(g_GameManager.stateFlags & 0x200) && g_GameManager.gui->IsMsgRunning() &&
        mgr->unknown0 % 3 != 2)
    {
        return 6;
    }
    return 1;
}

ReplayHeader *ValidateReplay(void *data, int size)
{
    BYTE *bytes;
    int sum;
    BYTE b;
    ReplayHeader *file;
    int i;
    ReplayHeader *decoded;

    file = (ReplayHeader *)data;
    if (file == NULL)
    {
        goto fail;
    }
    if (*(DWORD *)file->magic != *(DWORD *)"T9RP")
    {
        goto fail;
    }
    if (file->version != 2)
    {
        goto fail;
    }
    if (file->unknown6 != 0)
    {
        goto fail;
    }
    bytes = (BYTE *)&file->compressedSize;
    b = file->key;
    for (i = 0; i < file->size - 0x18; i++, bytes++)
    {
        *bytes -= b;
        b += 7;
    }
    sum = 0x3f000318;
    for (i = 0; i < file->size - 0x15; i++)
    {
        sum += (&file->key)[i];
    }
    if (sum != file->checksum)
    {
        goto fail;
    }
    decoded = (ReplayHeader *)g_MemoryManager.Alloc(file->dataSize + 0xc0 + (size - file->size), GLOBAL_H);
    memcpy(decoded, data, 0xc0);
    LzssDecompress((BYTE *)file + 0xc0, file->compressedSize, (BYTE *)decoded + 0xc0, file->dataSize);
    memcpy((BYTE *)decoded + file->dataSize + 0xc0, (BYTE *)data + file->size, size - file->size);
    if (g_App.CheckVersionFile(decoded->exeVersion, decoded->exeSize, decoded->exeChecksum) != 0)
    {
        goto failDecoded;
    }
    for (i = 0; i < 10; i++)
    {
        if (decoded->unknown20[i] != NULL)
        {
            decoded->unknown20[i] = (BYTE *)decoded->unknown20[i] + (int)decoded;
        }
        if (decoded->unknown48[i] != NULL)
        {
            decoded->unknown48[i] = (BYTE *)decoded->unknown48[i] + (int)decoded;
        }
        if (decoded->unknown70[i] != NULL)
        {
            decoded->unknown70[i] = (BYTE *)decoded->unknown70[i] + (int)decoded;
        }
        if (decoded->unknown98[i] != NULL)
        {
            decoded->unknown98[i] = (BYTE *)decoded->unknown98[i] + (int)decoded;
        }
    }
    g_MemoryManager.Free(data);
    return decoded;

fail:
    g_MemoryManager.Free(data);
    return NULL;

failDecoded:
    g_MemoryManager.Free(data);
    g_MemoryManager.Free(decoded);
    return NULL;
}

ReplayInfo::ReplayInfo()
{
}

// Removes the chain elements unknownElem158 and unknownElem15c, if set,
// and frees the blocks. Returns 0. Static, with the manager in eax
// (0x420a60).
static int FreeResources(ReplayManager *mgr)
{
    if (mgr->unknownElem158 != NULL)
    {
        g_Chain.Remove(mgr->unknownElem158);
        mgr->unknownElem158 = NULL;
    }
    if (mgr->unknownElem15c != NULL)
    {
        g_Chain.Remove(mgr->unknownElem15c);
        mgr->unknownElem15c = NULL;
    }
    g_MemoryManager.Free(mgr->unknown08);
    if (mgr->unknown110 != NULL)
    {
        g_MemoryManager.Free(mgr->unknown110);
    }
    return 0;
}

void ReplayManager::RemoveChain(ReplayManager *mgr)
{
    if (mgr != NULL)
    {
        FreeResources(mgr);
        g_Chain.Remove(mgr->calcElem);
        delete mgr;
    }
}

ReplayHeader::ReplayHeader()
{
}

ReplayManager *ReplayManager::RegisterChain(int mode, char *path)
{
    ReplayManager *mgr =
        (ReplayManager *)g_MemoryManager.Register(new ReplayManager(), sizeof(ReplayManager), "ReplayInf");

    memset(mgr, 0, sizeof(ReplayManager));
    mgr->mode = mode;
    mgr->unknown08 = NULL;
    switch (mode)
    {
    case 0:
        mgr->calcElem = g_Chain.CreateElem((ChainCallback)ReplayManager::OnUpdateMode0);
        mgr->calcElem->addedCallback = (ChainCallback)ReplayManager::AddedCallbackMode0;
        mgr->calcElem->arg = mgr;
        if (g_Chain.AddToCalcChain(mgr->calcElem, 0x1a) != 0)
        {
            return NULL;
        }
        mgr->unknownElem158 = NULL;
        mgr->unknownElem15c = g_Chain.CreateElem((ChainCallback)ReplayManager::OnUpdateMode0b);
        mgr->unknownElem15c->arg = mgr;
        g_Chain.AddToCalcChain(mgr->unknownElem15c, 7);
        ReplayManager::OnUpdateMode0b(mgr);
        break;
    case 1:
        mgr->path = path;
        mgr->calcElem = g_Chain.CreateElem((ChainCallback)ReplayManager::OnUpdateMode1);
        mgr->calcElem->addedCallback = (ChainCallback)ReplayManager::AddedCallbackMode1;
        mgr->calcElem->arg = mgr;
        if (g_Chain.AddToCalcChain(mgr->calcElem, 6) != 0)
        {
            return NULL;
        }
        mgr->unknownElem158 = g_Chain.CreateElem((ChainCallback)ReplayManager::OnUpdateMode1b);
        mgr->unknownElem158->arg = mgr;
        g_Chain.AddToCalcChain(mgr->unknownElem158, 0x1b);
        mgr->unknownElem15c = NULL;
        break;
    }
    return mgr;
}

void ReplayManager::RunAddedCallback(ReplayManager *mgr, int mode)
{
    switch (mode)
    {
    case 0:
        ReplayManager::AddedCallbackMode0(mgr);
        break;
    case 1:
        ReplayManager::AddedCallbackMode1(mgr);
        break;
    }
}
