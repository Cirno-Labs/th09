// cflags: -/O2 /O1 /Oi
// The GameManager object, from 0x41a6eb. It is built with /O1 rather than
// /O2: its functions are packed without alignment padding and clear
// memory with `and [mem], 0`.

#include <time.h>

#include "gamemanager.h"

BOOL GameManager::IsInPlayArea(float x, float y, float width, float height)
{
    if (x + width / 2.0f < -144.0f)
    {
        return FALSE;
    }
    if (144.0f < x - width / 2.0f)
    {
        return FALSE;
    }
    if (y + height / 2.0f < 0.0f)
    {
        return FALSE;
    }
    if (448.0f < y - height / 2.0f)
    {
        return FALSE;
    }
    return TRUE;
}

int GameManager::OnDraw(GameManager *mgr)
{
    if (mgr->isPaused)
    {
        mgr->isPaused = 2;
    }
    if (g_App.scene != 2)
    {
        return 3;
    }
    if ((mgr->stateFlags >> 5 & 3) == 1)
    {
        return 3;
    }
    if (mgr->unknown124 != 0)
    {
        return 3;
    }
    return 1;
}

void FormatDate(char *buf)
{
    time_t t;

    time(&t);
    strftime(buf, 10, "%y/%m/%d", localtime(&t));
}

void ChainElem::SetCallback(ChainCallback callback)
{
    this->callback = callback;
    this->addedCallback = NULL;
    this->deletedCallback = NULL;
}
