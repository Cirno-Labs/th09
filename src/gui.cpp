#include "gui.h"
#include "gamemanager.h"
#include "sprite.h"

int FreesResources()
{
    return g_App.freesResources;
}

int Gui::FreeResources(Gui *gui)
{
    if (FreesResources())
    {
        gui->msg.UnloadBgm(g_GameManager.sides[1].character * 10);
    }
    return 0;
}

void Gui::RemoveChain(Gui *gui)
{
    if (gui != NULL)
    {
        FreeResources(gui);
        g_Chain.Remove(gui->drawElem);
        g_Chain.Remove(gui->calcElem);
        delete gui;
    }
}

// Unloads ANM slot 7 if the side's front has unknownA67c set. Returns 0.
// Static, with the side in eax (0x418930).
static int FreeResources(GuiSide *side)
{
    if (side->unknownA67c != 0)
    {
        g_SpriteEngine->ReleaseAnm(7);
    }
    return 0;
}

void GuiSide::RemoveChain(GuiSide *side)
{
    if (side != NULL)
    {
        FreeResources(side);
        g_Chain.Remove(side->drawElem);
        g_Chain.Remove(side->calcElem);
        delete side;
    }
}
