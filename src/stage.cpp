#include "stage.h"

void Stage::CutChain(Stage *stage)
{
    if (stage != NULL)
    {
        g_Chain.Remove(stage->calcElem);
        g_Chain.Remove(stage->drawElem);
        g_Chain.Remove(stage->drawElem2);
        delete stage;
    }
}
