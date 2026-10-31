#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

void ClearViewport(DWORD color)
{
    if (g_SpriteEngine != NULL)
    {
        g_SpriteEngine->FlushQuadBatch();
    }
    g_App.SetView(2);
    ClearScreen(color);
}

int ScreenEffect::DeletedCallback(ScreenEffect *effect)
{
    effect->calcElem->deletedCallback = NULL;
    g_Chain.Remove(effect->drawElem);
    effect->drawElem = NULL;
    delete effect;
    return 0;
}

void ScreenEffect::Finish()
{
    this->finishing = 1;
    this->timer = 0;
}
