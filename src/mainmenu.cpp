// cflags: -/O2 -/Ob0 /O1 /Ob1 /Oi
// The title menu's object, from 0x422f17, built with /O1 as the
// GameManager's. It starts with the copies of the inline functions it
// calls but does not expand.

#include "mainmenu.h"
#include "ascii.h"
#include "midi.h"

void MainMenu::PlaySoundNow(int idx, int arg)
{
    g_SoundPlayer.PlaySoundByIdx(idx, arg);
    g_SoundPlayer.PlayQueuedSounds();
}

void MainMenu::SetState(int state)
{
    this->prevState = this->state;
    this->state = state;
    this->stateFrames = 0;
    this->unknown1b230 = 0;
    this->subState = 0;
    this->unknownC910 = 0;
}

void MainMenu::ReassignButton(ControllerMapping *mapping, short button, short replacement, int unused)
{
    if (mapping->shootButton == button)
    {
        mapping->shootButton = replacement;
    }
    if (mapping->bombButton == button)
    {
        mapping->bombButton = replacement;
    }
    if (mapping->focusButton == button)
    {
        mapping->focusButton = replacement;
    }
    if (mapping->upButton == button)
    {
        mapping->upButton = replacement;
    }
    if (mapping->downButton == button)
    {
        mapping->downButton = replacement;
    }
    if (mapping->leftButton == button)
    {
        mapping->leftButton = replacement;
    }
    if (mapping->rightButton == button)
    {
        mapping->rightButton = replacement;
    }
    if (mapping->menuButton == button)
    {
        mapping->menuButton = replacement;
    }
    if (mapping->skipButton == button)
    {
        mapping->skipButton = replacement;
    }
}

int MainMenu::ShowButtonDigits(Sprite *vm, short value)
{
    if (value < 0)
    {
        vm[0].flags &= ~2;
        vm[1].flags &= ~2;
    }
    else
    {
        this->anm->SetSprite(&vm[0], vm[0].baseSpriteIndex + value / 10 * 2);
        this->anm->SetSprite(&vm[1], vm[1].baseSpriteIndex + value % 10 * 2);
        vm[0].flags |= 2;
        vm[1].flags |= 2;
    }
    return 0;
}
