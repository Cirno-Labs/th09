// cflags: -/O2 -/Ob0 /O1 /Ob1 /Oi
// The GameManager object, from 0x41a6eb. It is built with /O1 rather than
// /O2: its functions are packed without alignment padding and clear
// memory with `and [mem], 0`. Unlike the rest of the game it expands
// inline functions (/Ob1), and memset is an intrinsic (/Oi).

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

void GameManager::ResetState()
{
    int i;

    for (i = 0; i < 3; i++)
    {
        this->sides[i].flags &= ~1;
    }
    this->unknownB0 = 0;
    this->unknownB8 = 2;
    this->unknownC0 = 0;
    this->unknownD0 = 0;
    this->unknownBc = 0;
    this->unknown110 = 0;
    this->unknown128 = 0;
    this->unknown12c = 0;
    this->unknown130 = 0;
    this->isPaused = 0;
    this->unknown13d = 0;
    this->unknown13e = 0;
    this->unknown380 = 0;
}

void RemoveGameManagerChain()
{
    if (g_GameManager.sides[0].stats->score >= 1000000000)
    {
        g_GameManager.sides[0].stats->score = 999999999;
    }
    g_GameManager.sides[0].stats->guiScore = g_GameManager.sides[0].stats->score;
    if (g_GameManager.sides[1].stats->score >= 1000000000)
    {
        g_GameManager.sides[1].stats->score = 999999999;
    }
    g_GameManager.sides[1].stats->guiScore = g_GameManager.sides[1].stats->score;
    g_App.gameSpeed = 1.0f;
    g_Chain.Remove(&g_GameManagerCalcChain);
    g_Chain.Remove(&g_GameManagerDrawChain);
}

WORD Rng::GetSeed()
{
    return this->seed;
}

void GameConfig::SetDefaults()
{
    memset(this, 0, sizeof(GameConfig));
    this->version = 0x90003;
    this->padThresholdX = 600;
    this->padThresholdY = 600;
    this->musicMode = 1;
    this->unknownAf = 1;
    this->unknownB0 = 1;
    this->unknownAc = 0;
    this->unknownAd = 0;
    this->windowed = 0;
    this->unknownB2 = 0;
    this->mappings[0][0] = g_PlayerInputs[0].mappings[0];
    this->mappings[0][1] = g_PlayerInputs[0].mappings[1];
    this->mappings[0][2] = g_PlayerInputs[0].mappings[2];
    this->mappings[1][0] = g_PlayerInputs[1].mappings[0];
    this->mappings[1][1] = g_PlayerInputs[1].mappings[1];
    this->mappings[1][2] = g_PlayerInputs[1].mappings[2];
    this->unknownB3 = 2;
    this->bgmVolume = 100;
    this->seVolume = 80;
    this->unknownB4 = 0;
    this->unknownB5 = 0;
    this->unknownB6 = 0;
    this->unknownB7 = 0;
    this->unknownB8 = 1;
    this->unknownB9 = 2;
}

GameConfig::GameConfig()
{
    this->SetDefaults();
}

void App::ResetSubThread()
{
    this->subThread = NULL;
    this->closeSubThread = 0;
    this->unknown6b8 = 0;
}

int GameManager::IsDemo()
{
    return this->stateFlags >> 1 & 1;
}

void GameManager::AddExtend()
{
    if (g_GameManager.sides[0].stats->lives < 7.0f)
    {
        g_GameManager.sides[0].stats->lives += 1.0f;
        g_SoundPlayer.PlaySoundByIdx(0x1c, 0);
    }
    g_GameManager.sides[0].stats->extendsGiven++;
    if (g_GameManager.mode == 1)
    {
        if (g_GameManager.sides[0].stats->extendsGiven >= 8)
        {
            g_GameManager.sides[0].stats->extendsGiven = 99999;
        }
    }
    else if (g_GameManager.sides[0].stats->extendsGiven >= 5)
    {
        g_GameManager.sides[0].stats->extendsGiven = 99999;
    }
}

int GameManager::RegisterChain()
{
    GameManager *mgr = &g_GameManager;

    mgr->frames = 0;
    g_GameManagerCalcChain.SetCallback((ChainCallback)GameManager::OnUpdate);
    g_GameManagerCalcChain.addedCallback = (ChainCallback)GameManager::AddedCallback;
    g_GameManagerCalcChain.deletedCallback = (ChainCallback)GameManager::DeletedCallback;
    g_GameManagerCalcChain.arg = mgr;
    if (g_Chain.AddToCalcChain(&g_GameManagerCalcChain, 2) != 0)
    {
        return -1;
    }
    g_GameManagerDrawChain.SetCallback((ChainCallback)GameManager::OnDraw);
    g_GameManagerDrawChain.arg = mgr;
    g_Chain.AddToDrawChain(&g_GameManagerDrawChain, 5);
    return 0;
}
