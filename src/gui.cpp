#include "gui.h"
#include "gamemanager.h"

int FreesResources()
{
    return g_App.freesResources;
}

int Gui::FreeResources(Gui *gui)
{
    if (FreesResources())
    {
        gui->msg.UnloadBgm(g_GameManager.sides[1].unknown20 * 10);
    }
    return 0;
}
