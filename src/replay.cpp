// cflags: -/GX
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "replay.h"
#include "gamemanager.h"
#include "gui.h"
#include "lzss.h"
#include "unsorted.h"

// The names SaveReplay writes: by character, by GameManager::mode, with
// the match modes from 2 on, and by difficulty (0x4a1c78, 0x4a1cb8,
// 0x4a1cd0).
extern char *g_ReplayCharacterNames[16];
extern char *g_ReplayModeNames[6];
extern char *g_ReplayDifficultyNames[5];

// As th08's FormatAndAdvance: formats into `buf` like sprintf and
// returns the end of what it wrote. Static, with `buf` in esi
// (0x4200c0).
static char *FormatAndAdvance(char *buf, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    return buf + strlen(buf);
}

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

int ReplayManager::OnUpdateMode0b(ReplayManager *mgr)
{
    int i;
    GameConfig *config;

    mgr->unknown162 = 0;
    mgr->seed = g_Rng.GetSeed();
    g_Rng.ResetGenerated();
    if (g_GameManager.unknown130)
    {
        mgr->unknown162 |= 0x100;
    }
    g_GameManager.unknown130 = 0;
    g_PlayerInputs[0].unknown2e = g_PlayerInputs[0].unknown2c;
    g_PlayerInputs[0].unknown2c = g_PlayerInputs[0].unknown0;
    g_PlayerInputs[1].unknown2e = g_PlayerInputs[1].unknown2c;
    g_PlayerInputs[1].unknown2c = g_PlayerInputs[1].unknown0;
    g_PlayerInputs[2].unknown2e = g_PlayerInputs[2].unknown2c;
    g_PlayerInputs[2].unknown2c = g_PlayerInputs[2].unknown0;
    g_PlayerInputs[0].UpdateRepeat2();
    g_PlayerInputs[1].UpdateRepeat2();
    g_PlayerInputs[2].UpdateRepeat2();
    for (i = 0, config = g_GameManager.config; i < 2; i++)
    {
        if (config != NULL && config->unknownB4[i] == 1)
        {
            if (g_PlayerInputs[i].IsHeld2(1))
            {
                g_PlayerInputs[i].unknown2a++;
                if (g_PlayerInputs[i].unknown2a >= 8)
                {
                    g_PlayerInputs[i].unknown2a = 8;
                }
            }
            else
            {
                g_PlayerInputs[i].unknown2a = 0;
            }
        }
    }
    return 1;
}

int ReplayManager::OnUpdateMode0(ReplayManager *mgr)
{
    DWORD flags = g_GameManager.stateFlags;

    if (!(flags & 4))
    {
        return 1;
    }
    if (g_App.GetNoScore())
    {
        return 1;
    }
    if (flags & 0x200)
    {
        if (mgr->unknown4 >= 3)
        {
            return 1;
        }
        mgr->unknown4++;
    }
    if (g_GameManager.sides[0].unknown28 == 0)
    {
        *mgr->inputCursors[0] = g_PlayerInputs[0].unknown2c;
    }
    if (g_GameManager.sides[1].unknown28 == 0)
    {
        *mgr->inputCursors[1] = g_PlayerInputs[1].unknown2c;
    }
    *mgr->inputCursors[2] = g_PlayerInputs[2].unknown2c;
    mgr->curChunk->count++;
    mgr->inputCursors[0]++;
    mgr->inputCursors[1]++;
    mgr->inputCursors[2]++;
    if (mgr->unknown0 % 30 == 0)
    {
        *mgr->extraCursor = g_App.roundedFps;
        mgr->extraCursor++;
    }
    if (mgr->curChunk->count >= 0xe0e)
    {
        mgr->curChunk->next =
            (ReplayChunk *)g_MemoryManager.Register(new ReplayChunk, sizeof(ReplayChunk), "ReplayBufLinkInf");
        mgr->curChunk = mgr->curChunk->next;
        mgr->inputCursors[0] = (WORD *)g_MemoryManager.Alloc(0x1c20, "rep data");
        mgr->inputCursors[1] = (WORD *)g_MemoryManager.Alloc(0x1c20, "rep data");
        mgr->inputCursors[2] = (WORD *)g_MemoryManager.Alloc(0x1c20, "rep data");
        mgr->extraCursor = (BYTE *)g_MemoryManager.Alloc(0x79, "rep data");
        mgr->curChunk->inputs[0] = mgr->inputCursors[0];
        mgr->curChunk->inputs[1] = mgr->inputCursors[1];
        mgr->curChunk->inputs[2] = mgr->inputCursors[2];
        mgr->curChunk->extra = mgr->extraCursor;
        mgr->curChunk->next = NULL;
        mgr->curChunk->count = 0;
        memset(mgr->inputCursors[0], 0, 0x1c20);
        memset(mgr->inputCursors[1], 0, 0x1c20);
        memset(mgr->inputCursors[2], 0, 0x1c20);
        memset(mgr->extraCursor, 0, 0x79);
    }
    mgr->unknown0++;
    return 1;
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

int ReplayManager::OnUpdateMode1(ReplayManager *mgr)
{
    int i;
    BYTE *types;

    if ((g_GameManager.stateFlags & 4) && !(g_GameManager.stateFlags & 0x200))
    {
        g_PlayerInputs[0].unknown2e = g_PlayerInputs[0].unknown2c;
        g_PlayerInputs[1].unknown2e = g_PlayerInputs[1].unknown2c;
        g_PlayerInputs[2].unknown2e = g_PlayerInputs[2].unknown2c;
        g_PlayerInputs[0].unknown2c = *mgr->inputCursors[0];
        g_PlayerInputs[1].unknown2c = *mgr->inputCursors[1];
        g_PlayerInputs[2].unknown2c = *mgr->inputCursors[2];
        g_PlayerInputs[0].UpdateRepeat2();
        g_PlayerInputs[1].UpdateRepeat2();
        g_PlayerInputs[2].UpdateRepeat2();
        types = g_GameManager.config->unknownB4;
        for (i = 0; i < 2; i++, types++)
        {
            if (*types == 1)
            {
                if (g_PlayerInputs[i].IsHeld2(1))
                {
                    g_PlayerInputs[i].unknown2a++;
                    if (g_PlayerInputs[i].unknown2a >= 8)
                    {
                        g_PlayerInputs[i].unknown2a = 8;
                    }
                }
                else
                {
                    g_PlayerInputs[i].unknown2a = 0;
                }
            }
        }
        mgr->inputCursors[0]++;
        mgr->inputCursors[1]++;
        mgr->inputCursors[2]++;
        if (mgr->unknown0 % 30 == 0)
        {
            g_App.roundedFps = (char)mgr->extraCursor[1] & 0x7f;
            g_App.fpsWarning = (char)mgr->extraCursor[1] >> 7;
            mgr->extraCursor++;
        }
        mgr->unknown0++;
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
        if (decoded->unknown20[0][i] != NULL)
        {
            decoded->unknown20[0][i] = (BYTE *)decoded->unknown20[0][i] + (int)decoded;
        }
        if (decoded->unknown20[1][i] != NULL)
        {
            decoded->unknown20[1][i] = (BYTE *)decoded->unknown20[1][i] + (int)decoded;
        }
        if (decoded->unknown20[2][i] != NULL)
        {
            decoded->unknown20[2][i] = (BYTE *)decoded->unknown20[2][i] + (int)decoded;
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

int ReplayManager::AddedCallbackMode1(ReplayManager *mgr)
{
    int size;
    int idx;
    int i;
    ReplayRound *round0;
    ReplayRound *round1;
    ReplayRound *round2;
    BYTE *extra;

    mgr->unknown0 = 0;
    if (mgr->unknown08 == NULL)
    {
        mgr->unknown08 = (ReplayHeader *)OpenPath(mgr->path, &size, !(g_GameManager.stateFlags >> 1 & 1));
        mgr->unknown08 = ValidateReplay(mgr->unknown08, size);
        if (mgr->unknown08 == NULL)
        {
            return -1;
        }
        mgr->unknown110 = NULL;
    }
    idx = g_GameManager.unknownFc;
    if (mgr->unknown08->unknown20[idx] == NULL)
    {
        return -1;
    }
    if (g_GameManager.IsMode0() || g_GameManager.IsMode1())
    {
        for (i = 0; i < idx; i++)
        {
            g_GameManager.unknownD8[((ReplayRound *)mgr->unknown08->unknown20[1][i])->character] = 1;
        }
    }
    round0 = (ReplayRound *)mgr->unknown08->unknown20[0][idx];
    round1 = (ReplayRound *)mgr->unknown08->unknown20[1][idx];
    round2 = (ReplayRound *)mgr->unknown08->unknown20[2][idx];
    extra = (BYTE *)mgr->unknown08->unknown98[idx];
    g_GameManager.sides[0].character = round0->character;
    g_GameManager.sides[1].character = round1->character;
    g_GameManager.sides[0].unknown28 = round0->unknown7;
    g_GameManager.sides[1].unknown28 = round1->unknown7;
    g_GameManager.sides[0].unknown2c = round0->unknownA;
    g_GameManager.sides[1].unknown2c = round1->unknownA;
    g_GameManager.stage = round0->stage;
    g_GameManager.unknown138 = round0->unknown9;
    g_GameManager.difficulty = mgr->unknown08->info.difficulty;
    g_GameManager.initialLives[0] = mgr->unknown08->unknown1e6[0];
    g_GameManager.initialLives[1] = mgr->unknown08->unknown1e6[1];
    g_GameManager.sides[0].unknown30 = mgr->unknown08->unknown1e8[0];
    g_GameManager.sides[1].unknown30 = mgr->unknown08->unknown1e8[1];
    g_GameManager.sides[0].stats->score = round0->score;
    g_GameManager.sides[1].stats->score = round1->score;
    g_GameManager.sides[0].stats->guiScore = round0->score;
    g_GameManager.sides[1].stats->guiScore = round1->score;
    g_GameManager.sides[0].stats->lives = round0->lives;
    g_GameManager.sides[1].stats->lives = round1->lives;
    g_GameManager.sides[0].stats->extendsGiven = round0->extendsGiven;
    mgr->inputCursors[0] = (WORD *)(round0 + 1);
    mgr->inputCursors[1] = (WORD *)(round1 + 1);
    mgr->inputCursors[2] = (WORD *)(round2 + 1);
    mgr->extraCursor = extra;
    *g_GameManager.config = mgr->unknown08->info.config;
    g_Rng.SetSeed(round0->rngSeed);
    mgr->unknown4 = 0;
    return 0;
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

void ReplayManager::Release(ReplayManager *mgr)
{
    ReplayChunk *chunk;
    ReplayChunk *cur;
    int i;
    int j;

    if (mgr == NULL)
    {
        return;
    }
    for (i = 0; i < 10; i++)
    {
        for (chunk = &mgr->chunks[i]; chunk != NULL;)
        {
            cur = chunk;
            if (chunk->inputs[0] != NULL)
            {
                g_MemoryManager.Free(chunk->inputs[0]);
            }
            if (chunk->inputs[1] != NULL)
            {
                g_MemoryManager.Free(chunk->inputs[1]);
            }
            if (chunk->inputs[2] != NULL)
            {
                g_MemoryManager.Free(chunk->inputs[2]);
            }
            if (chunk->extra != NULL)
            {
                g_MemoryManager.Free(chunk->extra);
            }
            chunk = chunk->next;
            memset(cur, 0, sizeof(ReplayChunk));
            if (cur != &mgr->chunks[i])
            {
                g_MemoryManager.Free(cur);
            }
        }
        if (!g_GameManager.IsReplay())
        {
            for (j = 0; j < 3; j++)
            {
                if (mgr->unknown08->unknown20[j][i] != NULL)
                {
                    g_MemoryManager.Free(mgr->unknown08->unknown20[j][i]);
                }
                mgr->unknown08->unknown20[j][i] = NULL;
            }
            if (mgr->unknown08->unknown98[i] != NULL)
            {
                g_MemoryManager.Free(mgr->unknown08->unknown98[i]);
            }
            mgr->unknown08->unknown98[i] = NULL;
        }
    }
    FreeResources(mgr);
    g_Chain.Remove(mgr->calcElem);
    delete mgr;
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

int ReplayManager::SaveReplay(ReplayManager *mgr, char *path, char *name)
{
    BYTE *dp2;
    BYTE kbVal;
    BYTE *bpCur;
    int checksum;
    ReplayUserHeader user;
    char userInfo[0x400];
    BYTE *rpyBytes;
    ReplayChunk *chunk;
    int chunkSize;
    char *p2;
    int allocSize;
    int mode;
    int j;
    int i;

    if (mgr != NULL && path != NULL)
    {
        ReplayHeader hX;
        int offsets;

        allocSize = sizeof(ReplayHeader) + 0x10000;
        for (i = 0; i < 10; i++)
        {
            if (hX.unknown20[0][i] != NULL)
            {
                allocSize += 0x60;
                for (chunk = &mgr->chunks[i]; chunk != NULL; chunk = chunk->next)
                {
                    allocSize += 0x54d9;
                }
            }
        }
        rpyBytes = (BYTE *)g_MemoryManager.Alloc(allocSize, "rep tmp");
        hX = *mgr->unknown08;
        offsets = sizeof(ReplayHeader);
        for (j = 0; j < 3; j++)
        {
            for (i = 0; i < 10; i++)
            {
                if (hX.unknown20[j][i] != NULL)
                {
                    memcpy(rpyBytes + offsets - 0xc0, hX.unknown20[j][i], 0x20);
                    hX.unknown20[j][i] = (void *)offsets;
                    offsets += 0x20;
                    for (chunk = &mgr->chunks[i]; chunk != NULL; chunk = chunk->next)
                    {
                        chunkSize = chunk->count * 2;
                        memcpy(rpyBytes + offsets - 0xc0, chunk->inputs[j], chunkSize);
                        offsets += chunkSize;
                    }
                }
            }
        }
        for (i = 0; i < 10; i++)
        {
            if (hX.unknown98[i] != NULL)
            {
                hX.unknown98[i] = (void *)offsets;
                for (chunk = &mgr->chunks[i]; chunk != NULL; chunk = chunk->next)
                {
                    chunkSize = chunk->count / 30;
                    memcpy(rpyBytes + offsets - 0xc0, chunk->extra, chunkSize);
                    offsets += chunkSize;
                }
            }
        }
        memset(userInfo, 0, sizeof(userInfo));
        user.magic = *(DWORD *)"USER";
        user.type = 0;
        p2 = userInfo;
        p2 = FormatAndAdvance(p2, "プレイヤー名\t%s\r\n", name);
        p2 = FormatAndAdvance(p2, "プレイ時刻\t%s\r\n", g_Hscr.date);
        p2 = FormatAndAdvance(p2, "難易度\t\t%s\r\n", g_ReplayDifficultyNames[g_GameManager.difficulty]);
        mode = g_GameManager.mode;
        if (mode == 2)
        {
            mode = g_GameManager.unknown114 + 2;
        }
        p2 = FormatAndAdvance(p2, "モード\t\t%s\r\n", g_ReplayModeNames[mode]);
        if (!g_GameManager.IsMode2())
        {
            p2 = FormatAndAdvance(p2, "使用キャラ\t\t%s\r\n", g_ReplayCharacterNames[g_GameManager.sides[0].character]);
        }
        else
        {
            p2 = FormatAndAdvance(p2, "対戦キャラ\t\t%s vs %s\r\n", g_ReplayCharacterNames[g_GameManager.sides[0].character],
                                  g_ReplayCharacterNames[g_GameManager.sides[1].character]);
        }
        p2 = FormatAndAdvance(p2, "初期体力\t\t%1.1f vs %1.1f\r\n", g_GameManager.initialLives[0] * 0.5f,
                              g_GameManager.initialLives[1] * 0.5f);
        p2 = FormatAndAdvance(p2, "ゲームのバージョン\t%s\r\n", "東方花映塚 ver 1.50a");
        user.size = strlen(userInfo) + 1 + 0xc;
        user.size += user.size & 1;
        hX.isSaved = 1;
        strcpy(hX.info.name, name);
        strcpy(hX.info.date, g_Hscr.date);
        hX.key = g_Rng.GetRandomU16InRange(0x80) + 0x40;
        hX.replayFormatVersion = 30;
        hX.info.randomByte = g_Rng.GetRandomU16InRange(0x100);
        hX.secondaryKey = g_Rng.GetRandomU16InRange(0x100);
        memcpy(rpyBytes, &hX.info, sizeof(ReplayHeader) - 0xc0);
        hX.dataSize = offsets - 0xc0;
        {
            BYTE *compressedData;
            int compSize;
            HANDLE file;
            DWORD writtenSize;
            compressedData = (BYTE *)LzssCompress(rpyBytes, hX.dataSize, &hX.compressedSize);
            g_MemoryManager.Free(rpyBytes);
            compSize = hX.compressedSize;
            bpCur = &hX.key;
            checksum = 0x3f000318;
            for (i = 0; i < 0xabU; i++, bpCur++)
            {
                checksum += *bpCur;
            }
            for (i = 0; i < compSize; i++)
            {
                checksum += compressedData[i];
            }
            hX.checksum = checksum;
            dp2 = (BYTE *)&hX.compressedSize;
            kbVal = hX.key;
            for (i = 0; i < 0xa8U; i++, dp2++)
            {
                *dp2 += kbVal;
                kbVal += 7;
            }
            dp2 = compressedData;
            for (i = 0; i < compSize; i++, dp2++)
            {
                *dp2 += kbVal;
                kbVal += 7;
            }
            hX.size = compSize + 0xc0;
            writtenSize = 0;
            file = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (file != INVALID_HANDLE_VALUE)
            {
                WriteFile(file, &hX, 0xc0, &writtenSize, NULL);
                WriteFile(file, compressedData, compSize, &writtenSize, NULL);
                WriteFile(file, &user, 0xc, &writtenSize, NULL);
                WriteFile(file, userInfo, user.size - 0xc, &writtenSize, NULL);
                CloseHandle(file);
                GlobalFree(compressedData);
            }
        }
    }
    return 0;
}

// The recording set-up, as th08's RecordAddedCallback: it creates the
// header on first use, then starts the round's three ReplayRound blocks
// (both sides' state) and frame-rate byte, and resets the round's chunk
// list to one chunk with fresh input buffers. The small blocks come from
// operator new itself, which the original pushes Register's other
// arguments before calling.
int ReplayManager::AddedCallbackMode0(ReplayManager *mgr)
{
    int idx;
    int i;
    ReplayRound *round0;
    ReplayRound *round1;
    ReplayRound *round2;
    BYTE *extra;
    ReplayChunk *chunk;
    ReplayChunk *cur;
    ReplayHeader *hdr;

    mgr->unknown0 = 0;
    mgr->unknown110 = NULL;
    hdr = mgr->unknown08;
    if (hdr == NULL)
    {
        mgr->unknown08 = (ReplayHeader *)g_MemoryManager.Register(new ReplayHeader, sizeof(ReplayHeader),
                                                                  "ReplayFileHeaderInf");
        hdr = mgr->unknown08;
        memset(hdr, 0, sizeof(ReplayHeader));
        hdr->isSaved = 0;
        hdr->unknown6 = 0;
        *(DWORD *)hdr->magic = *(DWORD *)"T9RP";
        hdr->version = 2;
        hdr->info.version = 0x150;
        hdr->info.versionLetter = 'a';
        strcpy(hdr->exeVersion, "0150a");
        hdr->exeSize = g_App.exeSize;
        hdr->exeChecksum = g_App.exeChecksum;
        hdr->info.difficulty = g_GameManager.difficulty;
        hdr->mode = g_GameManager.mode;
        hdr->unknown1e5 = g_GameManager.unknown114;
        hdr->unknown1e6[0] = g_GameManager.initialLives[0];
        hdr->unknown1e6[1] = g_GameManager.initialLives[1];
        mgr->unknown08->unknown1e8[0] = g_GameManager.sides[0].unknown30;
        mgr->unknown08->unknown1e8[1] = g_GameManager.sides[1].unknown30;
        *(DWORD *)hdr->info.name = *(DWORD *)"NO NAME";
        hdr->info.config = *g_GameManager.config;
        for (i = 0; i < 10; i++)
        {
            hdr->unknown20[0][i] = NULL;
            hdr->unknown20[1][i] = NULL;
            hdr->unknown20[2][i] = NULL;
            hdr->unknown98[i] = NULL;
        }
    }
    idx = g_GameManager.unknownFc;
    if (hdr->unknown20[0][idx] != NULL)
    {
        g_MemoryManager.Free(hdr->unknown20[0][idx]);
    }
    if (hdr->unknown20[1][idx] != NULL)
    {
        g_MemoryManager.Free(hdr->unknown20[1][idx]);
    }
    if (hdr->unknown20[2][idx] != NULL)
    {
        g_MemoryManager.Free(hdr->unknown20[2][idx]);
    }
    if (hdr->unknown98[idx] != NULL)
    {
        g_MemoryManager.Free(hdr->unknown98[idx]);
    }
    round0 = (ReplayRound *)g_MemoryManager.Register(operator new(sizeof(ReplayRound)), sizeof(ReplayRound), "ReplayFrameDataStartInf");
    hdr->unknown20[0][idx] = round0;
    round1 = (ReplayRound *)g_MemoryManager.Register(operator new(sizeof(ReplayRound)), sizeof(ReplayRound), "ReplayFrameDataStartInf");
    hdr->unknown20[1][idx] = round1;
    round2 = (ReplayRound *)g_MemoryManager.Register(operator new(sizeof(ReplayRound)), sizeof(ReplayRound), "ReplayFrameDataStartInf");
    hdr->unknown20[2][idx] = round2;
    extra = (BYTE *)g_MemoryManager.Register(operator new(sizeof(BYTE)), sizeof(BYTE), "ReplayFPSTblStartInf");
    hdr->unknown98[idx] = extra;
    memset(round0, 0, sizeof(ReplayRound));
    memset(round1, 0, sizeof(ReplayRound));
    memset(round2, 0, sizeof(ReplayRound));
    *extra = 0;
    round2->rngSeed = g_GameManager.rngSeed;
    round1->rngSeed = g_GameManager.rngSeed;
    round0->rngSeed = g_GameManager.rngSeed;
    round0->character = g_GameManager.sides[0].character;
    round1->character = g_GameManager.sides[1].character;
    round0->unknown7 = g_GameManager.sides[0].unknown28;
    round1->unknown7 = g_GameManager.sides[1].unknown28;
    round0->unknownA = g_GameManager.sides[0].unknown2c;
    round1->unknownA = g_GameManager.sides[1].unknown2c;
    round0->lives = g_GameManager.sides[0].stats->lives;
    round1->lives = g_GameManager.sides[1].stats->lives;
    round0->score = g_GameManager.sides[0].stats->score;
    round1->score = g_GameManager.sides[1].stats->score;
    round0->stage = g_GameManager.stage;
    round0->unknown9 = g_GameManager.unknown138;
    round0->extendsGiven = g_GameManager.sides[0].stats->extendsGiven;
    mgr->inputCursors[0] = (WORD *)g_MemoryManager.Alloc(0x1c20, "rep data");
    mgr->inputCursors[1] = (WORD *)g_MemoryManager.Alloc(0x1c20, "rep data");
    mgr->inputCursors[2] = (WORD *)g_MemoryManager.Alloc(0x1c20, "rep data");
    mgr->extraCursor = (BYTE *)g_MemoryManager.Alloc(0x79, "rep data");
    mgr->curChunk = &mgr->chunks[idx];
    for (chunk = mgr->curChunk; chunk != NULL;)
    {
        cur = chunk;
        if (chunk->inputs[0] != NULL)
        {
            g_MemoryManager.Free(chunk->inputs[0]);
        }
        if (chunk->inputs[1] != NULL)
        {
            g_MemoryManager.Free(chunk->inputs[1]);
        }
        if (chunk->inputs[2] != NULL)
        {
            g_MemoryManager.Free(chunk->inputs[2]);
        }
        if (chunk->extra != NULL)
        {
            g_MemoryManager.Free(chunk->extra);
        }
        chunk = chunk->next;
        memset(cur, 0, sizeof(ReplayChunk));
        if (cur != &mgr->chunks[idx])
        {
            g_MemoryManager.Free(cur);
        }
    }
    mgr->curChunk->inputs[0] = mgr->inputCursors[0];
    mgr->curChunk->inputs[1] = mgr->inputCursors[1];
    mgr->curChunk->inputs[2] = mgr->inputCursors[2];
    mgr->curChunk->extra = mgr->extraCursor;
    mgr->curChunk->next = NULL;
    mgr->curChunk->count = 0;
    memset(mgr->inputCursors[0], 0, 0x1c20);
    memset(mgr->inputCursors[1], 0, 0x1c20);
    memset(mgr->inputCursors[2], 0, 0x1c20);
    memset(mgr->extraCursor, 0, 0x79);
    return 0;
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
