// The file with SoundPlayer, the sound effects and wave music, from
// 0x43e170, as th08's.

#include "dsutil.h"
#include "unsorted.h"

// The original allocates in an inline from global.h, whose __FILE__ names
// the block.
#define GLOBAL_H "./system\\global.h"

// Finds chunk `name` among the RIFF chunks at `data`, `size` bytes of
// them. Returns its contents and sets *chunkSize, or returns NULL.
static char *FindRiffChunk(char *data, char *name, int *chunkSize, unsigned int size)
{
    while (size > 0)
    {
        *chunkSize = *(int *)(data + 4);
        if (strncmp(data, name, 4) == 0)
        {
            return data + 8;
        }
        size -= *chunkSize + 8;
        data += *chunkSize + 8;
    }
    return NULL;
}

int SoundPlayer::LoadBgmFormat(char *path)
{
    this->bgmFormat = (BgmFormat *)OpenPath(path, NULL, 0);
    return this->bgmFormat != NULL ? 0 : -1;
}

void SoundPlayer::PlaySoundByIdx(int idx, int arg)
{
    int i;
    int pri;

    pri = g_SoundInfos[idx].priority;
    for (i = 0; i < 0xc; i++)
    {
        if (this->queue[i] < 0)
        {
            break;
        }
        if (this->queue[i] == idx)
        {
            if (this->queueArgCounts[i] < 0x80)
            {
                this->queueArgs[i][this->queueArgCounts[i]] = arg;
                this->queueArgCounts[i]++;
            }
            return;
        }
    }
    if (i >= 0xc)
    {
        return;
    }
    this->queue[i] = idx;
    this->queuedSound[idx] = pri;
    this->queueArgs[i][0] = arg;
    this->queueArgCounts[i]++;
}

void SoundPlayer::QueueSoundPanned(int idx, float x)
{
    int i;
    int pan;
    int pri;

    pri = g_SoundInfos[idx].priority;
    pan = x * (1000.0f / 144.0f);
    for (i = 0; i < 0xc; i++)
    {
        if (this->queue[i] < 0)
        {
            break;
        }
        if (this->queue[i] == idx)
        {
            if (this->queueArgCounts[i] < 0x80)
            {
                this->queueArgs[i][this->queueArgCounts[i]] = pan;
                this->queueArgCounts[i]++;
            }
            return;
        }
    }
    if (i >= 0xc)
    {
        return;
    }
    this->queue[i] = idx;
    this->queuedSound[idx] = pri;
    this->queueArgs[i][0] = pan;
    this->queueArgCounts[i]++;
}

SoundPlayer::SoundPlayer()
{
    int i;

    memset(this, 0, sizeof(*this));
    for (i = 0; i < 0x80; i++)
    {
        this->queuedSound[i] = -1;
    }
}

int SoundPlayer::Initialize(HWND hwnd)
{
    DSBUFFERDESC params;
    LPVOID audioPtrTwo;
    DWORD size2;
    DWORD sizeOne;
    LPVOID audioPtr_1;
    WAVEFORMATEX formatInfo;
    int m;
    int j;

    memset(this, 0, sizeof(*this));
    for (j = 0; j < 0x80; j++)
    {
        this->queuedSound[j] = -1;
    }
    for (m = 0; m < 0xc; m++)
    {
        this->queue[m] = -1;
    }
    this->manager = new CSoundManager();
    if (this->manager->Initialize(hwnd, DSSCL_PRIORITY, 2, 44100, 16) < 0)
    {
        LogPrintf(&g_Log, "DirectSound オブジェクトの初期化が失敗したよ\r\n");
        if (this->manager != NULL)
        {
            delete this->manager;
            this->manager = NULL;
        }
        return -1;
    }
    this->dsound = this->manager->GetDirectSound();
    this->notifyThread = NULL;
    memset(&params, 0, sizeof(params));
    params.dwSize = sizeof(params);
    params.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE;
    params.dwBufferBytes = 0x8000;
    memset(&formatInfo, 0, sizeof(formatInfo));
    formatInfo.cbSize = 0;
    formatInfo.wFormatTag = WAVE_FORMAT_PCM;
    formatInfo.nChannels = 2;
    formatInfo.nSamplesPerSec = 44100;
    formatInfo.nAvgBytesPerSec = 44100 * 4;
    formatInfo.nBlockAlign = 4;
    formatInfo.wBitsPerSample = 16;
    params.lpwfxFormat = &formatInfo;
    if (this->dsound->CreateSoundBuffer(&params, &this->silence, NULL) < 0)
    {
        return -1;
    }
    if (this->silence->Lock(0, 0x8000, &audioPtr_1, &sizeOne, &audioPtrTwo, &size2, 0) < 0)
    {
        return -1;
    }
    memset(audioPtr_1, 0, 0x8000);
    this->silence->Unlock(audioPtr_1, sizeOne, audioPtrTwo, size2);
    this->silence->Play(0, 0, DSBPLAY_LOOPING);
    this->bgmVolume = 100;
    this->seVolume = 100;
    SetTimer(hwnd, 0, 250, NULL);
    this->hwnd = hwnd;
    LogPrintf(&g_Log, "DirectSound は正常に初期化されました\r\n");
    return 0;
}

int SoundPlayer::LoadSound(int index, char *path)
{
    char *cursor;
    DSBUFFERDESC params;
    char *chunk;
    int size;
    LPVOID audioPtrTwo;
    DWORD size2;
    DWORD sizeOne;
    LPVOID audioPtr_1;
    char *wavBuf;
    WAVEFORMATEX formatInfo;
    int totalSize2;

    if (!this->manager)
    {
        return 0;
    }
    if (this->soundBuffers[index] != NULL)
    {
        this->soundBuffers[index]->Release();
        this->soundBuffers[index] = NULL;
    }
    wavBuf = (char *)OpenPath(path, NULL, 0);
    cursor = wavBuf;
    if (cursor == NULL)
    {
        return -1;
    }
    if (strncmp(cursor, "RIFF", 4))
    {
        LogPrintf(&g_Log, "Wav ファイルじゃない %s\r\n", path);
        g_MemoryManager.Free(wavBuf);
        return -1;
    }
    cursor += 4;
    totalSize2 = *(int *)cursor;
    cursor += 4;
    if (strncmp(cursor, "WAVE", 4))
    {
        LogPrintf(&g_Log, "Wav ファイルじゃない? %s\r\n", path);
        g_MemoryManager.Free(wavBuf);
        return -1;
    }
    cursor += 4;
    chunk = FindRiffChunk(cursor, "fmt ", &size, totalSize2 - 12);
    if (chunk == NULL)
    {
        LogPrintf(&g_Log, "Wav ファイルじゃない? %s\r\n", path);
        g_MemoryManager.Free(wavBuf);
        return -1;
    }
    formatInfo = *(WAVEFORMATEX *)chunk;
    chunk = FindRiffChunk(cursor, "data", &size, totalSize2 - 12);
    if (chunk == NULL)
    {
        LogPrintf(&g_Log, "Wav ファイルじゃない? %s\r\n", path);
        g_MemoryManager.Free(wavBuf);
        return -1;
    }
    memset(&params, 0, sizeof(params));
    params.dwSize = sizeof(params);
    params.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE | DSBCAPS_GLOBALFOCUS;
    params.dwBufferBytes = size;
    params.lpwfxFormat = &formatInfo;
    if (this->dsound->CreateSoundBuffer(&params, &this->soundBuffers[index], NULL) < 0)
    {
        g_MemoryManager.Free(wavBuf);
        return -1;
    }
    if (this->soundBuffers[index]->Lock(0, size, &audioPtr_1, &sizeOne, &audioPtrTwo, &size2, 0) < 0)
    {
        g_MemoryManager.Free(wavBuf);
        return -1;
    }
    memcpy(audioPtr_1, chunk, sizeOne);
    if (size2 != 0)
    {
        memcpy(audioPtrTwo, chunk + sizeOne, size2);
    }
    this->soundBuffers[index]->Unlock(audioPtr_1, sizeOne, audioPtrTwo, size2);
    g_MemoryManager.Free(wavBuf);
    return 0;
}

int SoundPlayer::LoadSounds()
{
    int i;

    if (!this->manager)
    {
        return -1;
    }
    if (this->dsound == NULL)
    {
        return 0;
    }
    for (i = 0; i < 0xc; i++)
    {
        this->queue[i] = -1;
    }
    for (i = 0; i < 0x27; i++)
    {
        if (this->LoadSound(i, g_SoundPaths[i]) != 0)
        {
            LogPrintf(&g_Log, "error : Sound ファイルが読み込めない データを確認 %s\r\n", g_SoundPaths[i]);
            return -1;
        }
    }
    for (i = 0; i < ARRAY_SIZE(g_SoundInfos); i++)
    {
        this->dsound->DuplicateSoundBuffer(this->soundBuffers[g_SoundInfos[i].buffer], &this->sounds[i]);
        this->sounds[i]->SetCurrentPosition(0);
        this->sounds[i]->SetVolume(g_SoundInfos[i].volume);
    }
    return 0;
}
