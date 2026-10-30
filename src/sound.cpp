// The file with SoundPlayer, the sound effects and wave music, from
// 0x43e170, as th08's.

#include "dsutil.h"
#include "unsorted.h"

// The original allocates in an inline from global.h, whose __FILE__ names
// the block.
#define GLOBAL_H "./system\\global.h"

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
