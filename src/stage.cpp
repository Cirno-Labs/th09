#include "stage.h"
#include "gamemanager.h"
#include "unsorted.h"

void Stage::Reset()
{
    this->unknownCcc = 0;
}

int Stage::DeletedCallback(Stage *stage)
{
    if (FreesResources())
    {
        g_SpriteEngine->ReleaseAnm(4);
    }
    if (stage->unknown24 != NULL)
    {
        g_MemoryManager.Free(stage->unknown24);
        stage->unknown24 = NULL;
    }
    if (stage->unknown820 != NULL)
    {
        g_MemoryManager.Free(stage->unknown820);
        stage->unknown820 = NULL;
    }
    return 0;
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

Stage::Stage()
{
    memset(this, 0, sizeof(Stage));
}

Stage *Stage::RegisterChain(int sideIdx)
{
    Stage *stage = (Stage *)g_MemoryManager.Register(new Stage(), sizeof(Stage), "BGScrollInf");

    stage->unknownC68 = 0;
    stage->sideIdx = sideIdx;
    stage->side = &g_GameManager.sides[sideIdx];
    stage->otherSide = &g_GameManager.sides[1 - sideIdx];
    stage->calcElem = g_Chain.CreateElem((ChainCallback)Stage::OnUpdate);
    stage->calcElem->arg = stage;
    stage->calcElem->addedCallback = (ChainCallback)Stage::AddedCallback;
    stage->calcElem->deletedCallback = (ChainCallback)Stage::DeletedCallback;
    if (g_Chain.AddToCalcChain(stage->calcElem, sideIdx + 8) != 0)
    {
        return NULL;
    }
    stage->drawElem = g_Chain.CreateElem((ChainCallback)Stage::OnDraw);
    stage->drawElem2 = g_Chain.CreateElem((ChainCallback)Stage::OnDraw2);
    stage->drawElem->arg = stage;
    stage->drawElem2->arg = stage;
    g_Chain.AddToDrawChain(stage->drawElem, sideIdx + 6);
    g_Chain.AddToDrawChain(stage->drawElem2, sideIdx + 8);
    return stage;
}
