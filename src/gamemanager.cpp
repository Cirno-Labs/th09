// cflags: -/O2 /O1 /Oi
// The GameManager object, from 0x41a6eb. It is built with /O1 rather than
// /O2: its functions are packed without alignment padding and clear
// memory with `and [mem], 0`.

#include <time.h>

#include "gamemanager.h"

void ChainElem::SetCallback(ChainCallback callback)
{
    this->callback = callback;
    this->addedCallback = NULL;
    this->deletedCallback = NULL;
}
