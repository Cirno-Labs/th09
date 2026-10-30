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

void MainMenu::SkipClosedEntries(int idx, int delta, char *entries, int count)
{
    if (g_Network->unknownA8 != 0)
    {
        return;
    }
    int *cursor = &this->cursors[idx];
    while (!g_PlayRecord.openEntries[entries[*cursor]])
    {
        *cursor += delta;
        if (*cursor < 0)
        {
            *cursor += count;
        }
        if (*cursor >= count)
        {
            *cursor -= count;
        }
    }
}

void MainMenu::SkipClosedEntries2(int idx, int delta, char *entries, int count)
{
    if (g_Network->unknownA8 != 0)
    {
        return;
    }
    int *cursor = &this->cursors[idx];
    while (!g_PlayRecord.openEntries2[entries[*cursor]])
    {
        *cursor += delta;
        if (*cursor < 0)
        {
            *cursor += count;
        }
        if (*cursor >= count)
        {
            *cursor -= count;
        }
    }
}

void MainMenu::SkipClosedEntries3(int idx, int delta, char *entries, int count)
{
    if (g_Network->unknownA8 != 0)
    {
        return;
    }
    int *cursor = &this->cursors[idx];
    while (!g_PlayRecord.openEntries3[entries[*cursor]])
    {
        *cursor += delta;
        if (*cursor < 0)
        {
            *cursor += count;
        }
        if (*cursor >= count)
        {
            *cursor -= count;
        }
    }
}

void MainMenu::ShowHalves(int left, int right)
{
    this->anm->SetSprite(&this->sprites[142], left == 0 ? 0xc5 : left == 1 ? 0xc4 : 0xc3);
    left -= 2;
    this->anm->SetSprite(&this->sprites[143], left <= 0 ? 0xc5 : left == 1 ? 0xc4 : 0xc3);
    left -= 2;
    this->anm->SetSprite(&this->sprites[144], left <= 0 ? 0xc5 : left == 1 ? 0xc4 : 0xc3);
    left -= 2;
    this->anm->SetSprite(&this->sprites[145], left <= 0 ? 0xc5 : left == 1 ? 0xc4 : 0xc3);
    left -= 2;
    this->anm->SetSprite(&this->sprites[146], left <= 0 ? 0xc5 : left == 1 ? 0xc4 : 0xc3);
    this->anm->SetSprite(&this->sprites[148], right == 0 ? 0xc5 : right == 1 ? 0xc4 : 0xc3);
    int n = right - 2;
    this->anm->SetSprite(&this->sprites[149], n <= 0 ? 0xc5 : n == 1 ? 0xc4 : 0xc3);
    n -= 2;
    this->anm->SetSprite(&this->sprites[150], n <= 0 ? 0xc5 : n == 1 ? 0xc4 : 0xc3);
    n -= 2;
    this->anm->SetSprite(&this->sprites[151], n <= 0 ? 0xc5 : n == 1 ? 0xc4 : 0xc3);
    n -= 2;
    this->anm->SetSprite(&this->sprites[152], n <= 0 ? 0xc5 : n == 1 ? 0xc4 : 0xc3);
}

void MidiPlayer::Restart(int slot)
{
    this->Stop();
    this->ParseFile(slot);
    this->Play();
}

void App::ClearUnknown77c()
{
    this->unknown77c = 0;
    this->unknown784 = 0;
    this->unknown790 = 0;
    this->unknown788 = 0;
    this->unknown78c = 0;
}

void Sprite::SetMoveInterp(int frames, BYTE easing, D3DXVECTOR3 *from, D3DXVECTOR3 *to)
{
    this->interpDuration[0].current = 0;
    this->interpDuration[0].currentF = 0.0f;
    this->interpDuration[0].previous = -999999;
    Timer *frame = &this->interpFrame[0];
    frame->current = frames;
    frame->currentF = frames;
    frame->previous = -999999;
    this->interpEasing = easing;
    this->moveStart = *from;
    this->moveEnd = *to;
}

void Sprite::SetColorInterp(int frames, BYTE easing, D3DCOLOR from, D3DCOLOR to)
{
    this->interpDuration[1].current = 0;
    this->interpDuration[1].currentF = 0.0f;
    this->interpDuration[1].previous = -999999;
    Timer *frame = &this->interpFrame[1];
    frame->current = frames;
    frame->currentF = frames;
    frame->previous = -999999;
    this->colorInterpEasing = easing;
    ((BYTE *)&this->colorTween.y)[2] = from >> 16 & 0xff;
    ((BYTE *)&this->colorTween.y)[1] = from >> 8 & 0xff;
    ((BYTE *)&this->colorTween.y)[0] = from & 0xff;
    ((BYTE *)&this->colorTween.z)[2] = to >> 16 & 0xff;
    ((BYTE *)&this->colorTween.z)[1] = to >> 8 & 0xff;
    ((BYTE *)&this->colorTween.z)[0] = to & 0xff;
}

void SoundPlayer::UnPause()
{
    this->QueueSoundCommand(7, 0, "UnPause");
}

int PlayRecord::GetClearFlags(char character, int difficulty)
{
    if (difficulty >= 0)
    {
        return this->clearFlags[character][difficulty];
    }
    return this->clearFlags[character][0] | this->clearFlags[character][1] | this->clearFlags[character][2] |
           this->clearFlags[character][3];
}

void AsciiText::SetScale(float x, float y)
{
    this->scaleX = x;
    this->scaleY = y;
}

int MainMenu::DrawState12()
{
    if (this->subState == 1)
    {
        int idx = this->unknown11b74 + 0x9f;
        D3DXVECTOR3 savedPos = this->sprites[idx].pos;
        D3DCOLOR savedColor = this->sprites[idx].color;
        this->sprites[idx].color = 0xffffffff;
        this->sprites[idx].pos.x = 208.0f;
        this->sprites[idx].pos.y = 293.0f;
        g_SpriteEngine->DrawUnrotated(&this->sprites[idx]);
        this->sprites[idx].pos = savedPos;
        this->sprites[idx].color = savedColor;
        for (int i = 0; i < 8; i++)
        {
            g_SpriteEngine->DrawUnrotated(&this->unknown19d04[i]);
        }
    }
    return 1;
}

int MainMenu::MoveCursor(int count)
{
    if (count == 0)
    {
        return 0;
    }
    if (g_PlayerInputs[2].IsPressed(0x10))
    {
        this->cursor--;
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        if (this->cursor < 0)
        {
            this->cursor = count - 1;
        }
        if (this->cursor >= count)
        {
            this->cursor = 0;
        }
        return -1;
    }
    if (g_PlayerInputs[2].IsPressed(0x20))
    {
        this->cursor++;
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        if (this->cursor < 0)
        {
            this->cursor = count - 1;
        }
        if (this->cursor >= count)
        {
            this->cursor = 0;
        }
        return 1;
    }
    return 0;
}

int MainMenu::MoveCursorLR(int count)
{
    if (count == 0)
    {
        return 0;
    }
    if (g_PlayerInputs[2].IsPressed(0x40))
    {
        this->cursor--;
        if (this->cursor < 0)
        {
            this->cursor += count;
        }
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        return -1;
    }
    if (g_PlayerInputs[2].IsPressed(0x80))
    {
        this->cursor++;
        if (this->cursor >= count)
        {
            this->cursor -= count;
        }
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        return 1;
    }
    return 0;
}

int MainMenu::MoveCursor4(int count)
{
    if (count == 0)
    {
        return 0;
    }
    if (g_PlayerInputs[2].IsPressed(0x40))
    {
        this->cursor--;
        if (this->cursor < 0)
        {
            this->cursor += count;
        }
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        return -1;
    }
    if (g_PlayerInputs[2].IsPressed(0x80))
    {
        this->cursor++;
        if (this->cursor >= count)
        {
            this->cursor -= count;
        }
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        return 1;
    }
    if (g_PlayerInputs[2].IsPressed(0x10))
    {
        this->cursor--;
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        if (this->cursor < 0)
        {
            this->cursor = count - 1;
        }
        if (this->cursor >= count)
        {
            this->cursor = 0;
        }
        return -1;
    }
    if (g_PlayerInputs[2].IsPressed(0x20))
    {
        this->cursor++;
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        if (this->cursor < 0)
        {
            this->cursor = count - 1;
        }
        if (this->cursor >= count)
        {
            this->cursor = 0;
        }
        return 1;
    }
    return 0;
}

int MainMenu::MovePlayerCursor(int idx, int count)
{
    if (count == 0)
    {
        return 0;
    }
    if (g_PlayerInputs[idx].IsPressed(0x40))
    {
        this->cursors[idx]--;
        if (this->cursors[idx] < 0)
        {
            this->cursors[idx] += count;
        }
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        return -1;
    }
    if (g_PlayerInputs[idx].IsPressed(0x80))
    {
        this->cursors[idx]++;
        if (this->cursors[idx] >= count)
        {
            this->cursors[idx] -= count;
        }
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        return 1;
    }
    if (g_PlayerInputs[idx].IsPressed(0x10))
    {
        this->cursors[idx]--;
        if (this->cursors[idx] < 0)
        {
            this->cursors[idx] += count;
        }
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        return -1;
    }
    if (g_PlayerInputs[idx].IsPressed(0x20))
    {
        this->cursors[idx]++;
        if (this->cursors[idx] >= count)
        {
            this->cursors[idx] -= count;
        }
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        return 1;
    }
    return 0;
}

int MainMenu::MovePlayerCursor2(int player, int idx, int count)
{
    if (count == 0)
    {
        return 0;
    }
    if (g_PlayerInputs[player].IsPressed(0x40))
    {
        this->cursors[idx]--;
        if (this->cursors[idx] < 0)
        {
            this->cursors[idx] += count;
        }
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        return -1;
    }
    if (g_PlayerInputs[player].IsPressed(0x80))
    {
        this->cursors[idx]++;
        if (this->cursors[idx] >= count)
        {
            this->cursors[idx] -= count;
        }
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        return 1;
    }
    if (g_PlayerInputs[player].IsPressed(0x10))
    {
        this->cursors[idx]--;
        if (this->cursors[idx] < 0)
        {
            this->cursors[idx] += count;
        }
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        return -1;
    }
    if (g_PlayerInputs[player].IsPressed(0x20))
    {
        this->cursors[idx]++;
        if (this->cursors[idx] >= count)
        {
            this->cursors[idx] -= count;
        }
        g_SoundPlayer.PlaySoundByIdx(12, 0);
        return 1;
    }
    return 0;
}
