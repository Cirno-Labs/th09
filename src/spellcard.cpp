#include "spellcard.h"
#include "gamemanager.h"
#include "gui.h"

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
