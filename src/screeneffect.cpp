#include "screeneffect.h"

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
