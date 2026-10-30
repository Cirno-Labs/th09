// cflags: -/O2 /O1
// An object built with /O1 rather than /O2, from 0x41a6eb: its functions
// are packed without alignment padding and clear memory with
// `and [mem], 0`.

#include "chain.h"

void ChainElem::SetCallback(ChainCallback callback)
{
    this->callback = callback;
    this->addedCallback = NULL;
    this->deletedCallback = NULL;
}
