// cflags: -/O2 -/Ob0 /O1 /Ob1 /Oi
// The GameManager object, from 0x41a6eb. It is built with /O1 rather than
// /O2: its functions are packed without alignment padding and clear
// memory with `and [mem], 0`. Unlike the rest of the game it expands
// inline functions (/Ob1), and memset is an intrinsic (/Oi).

#include <time.h>

#include "gamemanager.h"
#include "ascii.h"
#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "exattack.h"
#include "gui.h"
#include "player.h"
#include "replay.h"
#include "spellcard.h"
#include "stage.h"

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
    if (mgr->loadingThreadState != 0)
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
    this->isInContinueMenu = 0;
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

void GameManager::ResetRound()
{
    int i;

    g_GameManager.ResetState();
    g_AsciiText.Initialize();
    g_GameManager.gui->Reset();
    g_GameManager.exAttackManager->Reset();
    for (i = 0; i < 2; i++)
    {
        g_GameManager.sides[i].flags &= ~1;
        g_GameManager.sides[i].stage->Reset();
        g_GameManager.sides[i].player->Reset();
        g_GameManager.sides[i].bulletManager->Reset();
        g_GameManager.sides[i].effectManager->Reset();
        g_GameManager.sides[i].enemyManager->Reset();
        g_GameManager.sides[i].spellCard->Reset();
        g_GameManager.sides[i].gui->Reset();
    }
    g_GameManager.sides[2].effectManager->Reset();
    if (g_GameManager.mode == 2)
    {
        g_GameManager.unknownB4 -= 8;
        if (g_GameManager.unknownB4 < g_GameManager.unknown100)
        {
            g_GameManager.unknownB4 = g_GameManager.unknown100;
        }
    }
    else
    {
        switch (g_GameManager.difficulty)
        {
        case 0:
            g_GameManager.unknownB4 = g_GameManager.unknownFc / 3 + 1;
            g_GameManager.unknownC8 = g_GameManager.unknownFc / 2 + 6;
            g_GameManager.unknownC4 = (g_GameManager.unknown100 + 3) * 600;
            break;
        case 1:
            g_GameManager.unknownB4 = g_GameManager.unknownFc + 1;
            g_GameManager.unknownC8 = g_GameManager.unknownFc + 10;
            g_GameManager.unknownC4 = (g_GameManager.unknown100 + 2) * 600;
            break;
        case 2:
            g_GameManager.unknownB4 = g_GameManager.unknownFc + 3;
            g_GameManager.unknownC8 = g_GameManager.unknownFc / 2 + 15;
            g_GameManager.unknownC4 = g_GameManager.unknown100 * 600 + 900;
            break;
        case 3:
            g_GameManager.unknownB4 = g_GameManager.unknownFc + 6;
            g_GameManager.unknownC8 = g_GameManager.unknownFc / 2 + 17;
            g_GameManager.unknownC4 = g_GameManager.unknown100 * 600 + 900;
            break;
        case 4:
            g_GameManager.unknownB4 = 18;
            g_GameManager.unknownC8 = 22;
            g_GameManager.unknownC4 = (g_GameManager.unknown100 + 1) * 600;
            break;
        }
        g_GameManager.unknownB4 -= g_GameManager.unknown100 * 2;
        if (g_GameManager.unknownB4 < 1)
        {
            g_GameManager.unknownB4 = 1;
        }
    }
    g_GameManager.unknownB0 = 0;
    g_GameManager.frames = 0;
    g_GameManager.unknownCc -= 1000;
    if (g_GameManager.unknownCc < 0)
    {
        g_GameManager.unknownCc = 0;
    }
}

int GameManager::AddedCallback(GameManager *mgr)
{
    if (g_App.scene != 3 && g_App.scene != 12 && g_App.scene != 10)
    {
        g_App.loadsResources = 1;
    }
    else
    {
        g_App.loadsResources = 0;
    }
    g_GameManager.loadingThreadState = 1;
    if (g_App.prevScene == 1)
    {
        D3DXVECTOR3 pos(500.0f, 440.0f, 0.0f);
        g_App.StartLoadingSpritesOverScreen(&pos);
        g_App.StartLoadingScreenEffect(0);
    }
    else
    {
        D3DXVECTOR3 pos(280.0f, 430.0f, 0.0f);
        g_App.StartLoadingSpritesOverScreen(&pos);
    }
    if (mgr->stageEndState >= 2)
    {
        mgr->stageEndState = 1;
    }
    g_App.StartSubThread((LPTHREAD_START_ROUTINE)GameManager::LoadingThread, NULL);
    g_App.AddTotalPlayTime();
    g_App.AddPlayTime();
    return 0;
}

int GameManager::DeletedCallback(GameManager *mgr)
{
    int i;

    g_GameManager.sides[0].unknown24 = g_GameManager.sides[0].character;
    g_GameManager.sides[1].unknown24 = g_GameManager.sides[1].character;
    g_GameManager.unknownF8 = g_GameManager.unknownF4;
    mgr->stateFlags &= ~4;
    if (g_App.scene != 3 && g_App.scene != 12 && g_App.scene != 10)
    {
        g_App.freesResources = 1;
    }
    else
    {
        g_App.freesResources = 0;
    }
    memset(g_Unknown4d9698, 0, sizeof(g_Unknown4d9698));
    if (g_GameManager.sides[0].stats != NULL)
    {
        g_Hscr.score = g_GameManager.sides[0].stats->score;
        g_Hscr.character = g_GameManager.sides[0].character;
        g_Hscr.difficulty = g_GameManager.difficulty;
        g_Hscr.unknown2b = g_GameManager.sides[0].stats->unknown14;
        g_Hscr.slowdown = 0.0f;
        g_Hscr.magic = 0x52435348;
        g_Hscr.unknown17 = 0;
        g_Hscr.size2 = 0x2c;
        g_Hscr.size = 0x2c;
        g_Hscr.version = 2;
        g_Hscr.isNew = 0;
    }
    if ((g_GameManager.stateFlags >> 3 & 1) && g_App.freesResources)
    {
        ReplayManager::RemoveChain(mgr->replayManager);
        mgr->replayManager = NULL;
    }
    for (i = 0; i < 2; i++)
    {
        Stage::RemoveChain(mgr->sides[i].stage);
        mgr->sides[i].stage = NULL;
        Player::RemoveChain(mgr->sides[i].player);
        mgr->sides[i].player = NULL;
        BulletManager::RemoveChain(mgr->sides[i].bulletManager);
        mgr->sides[i].bulletManager = NULL;
        EffectManager::RemoveChain(mgr->sides[i].effectManager);
        mgr->sides[i].effectManager = NULL;
        EnemyManager::RemoveChain(mgr->sides[i].enemyManager);
        mgr->sides[i].enemyManager = NULL;
        SpellCard::RemoveChain(mgr->sides[i].spellCard);
        mgr->sides[i].spellCard = NULL;
        GuiSide::RemoveChain(mgr->sides[i].gui);
        mgr->sides[i].gui = NULL;
    }
    Gui::RemoveChain(mgr->gui);
    mgr->gui = NULL;
    ExAttackManager::RemoveChain(mgr->exAttackManager);
    mgr->exAttackManager = NULL;
    EffectManager::RemoveChain(mgr->sides[2].effectManager);
    mgr->sides[2].effectManager = NULL;
    if (mgr->unknownE8 != NULL)
    {
        delete mgr->unknownE8;
        mgr->unknownE8 = NULL;
    }
    while (g_SoundPlayer.PlayQueuedSounds())
    {
    }
    g_GameManager.frames = 0;
    return 0;
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

void GameManager::Init()
{
    memset(this, 0, sizeof(GameManager));
    this->SetPlayArea();
    this->unknown13f = 0;
    this->unknown378 = 10;
    this->unknown37c = 10;
}
