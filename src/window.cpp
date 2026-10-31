// The game's window and Direct3D set-up, as th08's GameWindow object.

#include "unsorted.h"

int CreateD3d()
{
    g_App.d3d = Direct3DCreate8(220);
    if (g_App.d3d == NULL)
    {
        LogError(&g_Log, "Direct3D オブジェクトは何故か作成出来なかった\r\n");
        return 1;
    }
    return 0;
}
