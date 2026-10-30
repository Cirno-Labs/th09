#include "spellcard.h"
#include "effect.h"
#include "gamemanager.h"
#include "gui.h"
#include "player.h"

void Player::StartClearCircle1()
{
    this->AddHitCircle(&this->pos, 0.0f, 4.0f, 0x30, 0);
    this->AddKind4Circle(&this->pos, 0.0f, 4.0f, 2, 0x30, 0);
    this->SetUnknown0(3);
    this->unknown303c8 = 0x30;
    this->side->effectManager->SpawnParticles(0xd, &this->pos, 1, 0xffffffff);
}

void Player::StartClearCircle2()
{
    this->AddHitCircle(&this->pos, 0.0f, 4.0f, 0x40, 0);
    this->AddKind4Circle(&this->pos, 0.0f, 4.0f, 2, 0x40, 0);
    this->SetUnknown0(3);
    this->unknown303c8 = 0x40;
    this->side->effectManager->SpawnParticles(0xe, &this->pos, 1, 0xffffffff);
}

void Player::StartClearCircle3()
{
    this->AddHitCircle(&this->pos, 0.0f, 3.5f, 0x80, 0);
    this->AddKind4Circle(&this->pos, 0.0f, 3.5f, 2, 0x80, 0);
    this->SetUnknown0(3);
    this->unknown303c8 = 0x80;
    this->side->effectManager->SpawnParticles(0xf, &this->pos, 1, 0xffffffff);
}

// Calls the spell card's callbacks unknown1330 and unknown133c, if set.
// Returns 0. Static, with the spell card in esi (0x403d60).
static int RunEndCallbacks(SpellCard *card)
{
    if (card->unknown1330 != NULL)
    {
        card->unknown1330(card);
    }
    if (card->unknown133c != NULL)
    {
        card->unknown133c(card);
    }
    return 0;
}

void SpellCard::Reset()
{
    this->unknown14 = g_GameManager.gui->unknown08;
}

int SpellCard::AddedCallback(SpellCard *card)
{
    card->Reset();
    return 0;
}

void SpellCard::RemoveChain(SpellCard *card)
{
    if (card != NULL)
    {
        RunEndCallbacks(card);
        g_Chain.Remove(card->calcElem);
        g_Chain.Remove(card->drawElem);
        delete card;
    }
}

SpellCard *SpellCard::RegisterChain(int sideIdx)
{
    SpellCard *card =
        (SpellCard *)g_MemoryManager.Register(new SpellCard(), sizeof(SpellCard), "CardAttackInf");

    memset(card, 0, sizeof(SpellCard));
    card->sideIdx = sideIdx;
    card->side = &g_GameManager.sides[sideIdx];
    card->otherSide = &g_GameManager.sides[1 - sideIdx];
    card->calcElem = g_Chain.CreateElem((ChainCallback)SpellCard::OnUpdate);
    card->calcElem->addedCallback = (ChainCallback)SpellCard::AddedCallback;
    card->calcElem->arg = card;
    if (g_Chain.AddToCalcChain(card->calcElem, sideIdx + 0x11) != 0)
    {
        return NULL;
    }
    card->drawElem = g_Chain.CreateElem((ChainCallback)SpellCard::OnDraw);
    card->drawElem->arg = card;
    g_Chain.AddToDrawChain(card->drawElem, sideIdx + 0x18);
    return card;
}
