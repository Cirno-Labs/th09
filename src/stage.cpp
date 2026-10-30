#include "stage.h"

void Stage::Reset()
{
    this->unknownCcc = 0;
}

void Stage::RemoveChain(Stage *stage)
{
    if (stage != NULL)
    {
        g_Chain.Remove(stage->calcElem);
        g_Chain.Remove(stage->drawElem);
        g_Chain.Remove(stage->drawElem2);
        delete stage;
    }
}
