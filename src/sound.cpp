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
