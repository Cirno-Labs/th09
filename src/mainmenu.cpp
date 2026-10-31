// cflags: -/O2 -/Ob0 /O1 /Ob1
// The title menu's object, from 0x422f17, built with /O1 as the
// GameManager's. It starts with the copies of the inline functions it
// calls but does not expand.

#include "mainmenu.h"
#include "ascii.h"
#include "midi.h"

// The menu, for LoadingThread. The original keeps it in this object
// only (0x4ac8c0).
static MainMenu *g_MainMenu;

MusicTune::MusicTune()
{
    memset(this, 0, sizeof(MusicTune));
}

void MainMenu::PlaySoundNow(int idx, int arg)
{
    g_SoundPlayer.PlaySoundByIdx(idx, arg);
    g_SoundPlayer.PlayQueuedSounds();
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

int MainMenu::DrawState11()
{
    D3DXVECTOR3 pos;
    int i;
    int slot;
    ReplayHeader *replay;
    char buf[0x100];

    if (this->subState == 1)
    {
        pos.x = 32.0f;
        pos.y = 90.0f;
        pos.z = 0.0f;
        for (i = 0; i < 50; i++)
        {
            if (i == 25)
            {
                pos.x = 352.0f;
                pos.y = 90.0f;
            }
            g_AsciiText.color = i == this->cursor ? 0xffffffff : 0xff808080;
            if (i < 25)
            {
                g_AsciiText.AddFormat(&pos, "No.%.2d %8s %8s", i + 1,
                                      *(int *)this->replays[i].magic != 0 ? this->replays[i].info.name : "--------",
                                      *(int *)this->replays[i].magic != 0 ? this->replays[i].info.date : "--/--/--");
            }
            else
            {
                g_AsciiText.AddFormat(&pos, "%c%c%c%c  %8s %8s",
                                      *(int *)this->replays[i].magic != 0 ? this->replayPaths[i][15] : 'u',
                                      *(int *)this->replays[i].magic != 0 ? this->replayPaths[i][16] : 's',
                                      *(int *)this->replays[i].magic != 0 ? this->replayPaths[i][17] : 'e',
                                      *(int *)this->replays[i].magic != 0 ? this->replayPaths[i][18] : 'r',
                                      *(int *)this->replays[i].magic != 0 ? this->replays[i].info.name : "--------",
                                      *(int *)this->replays[i].magic != 0 ? this->replays[i].info.date : "--/--/--");
            }
            pos.y += 13.0f;
        }
    }
    else if (this->subState == 2)
    {
        replay = &this->replays[this->replaySlot];
        slot = this->replaySlot;
        pos.x = 32.0f;
        if (slot >= 25)
        {
            pos.x = 352.0f;
        }
        pos.y = slot * 13.0f + 90.0f;
        pos.z = 0.0f;
        D3DXVECTOR3 end(216.5f, 160.0f, 0.0f);
        if (this->unknown1b230 < 10)
        {
            pos = pos + (end - pos) * (float)this->unknown1b230 * 0.1f;
        }
        else
        {
            pos = end;
        }
        if (slot < 25)
        {
            g_AsciiText.AddFormat(&pos, "No.%.2d %8s %8s", slot + 1,
                                  *(int *)replay->magic != 0 ? replay->info.name : "--------",
                                  *(int *)replay->magic != 0 ? replay->info.date : "--/--/--");
        }
        else
        {
            g_AsciiText.AddFormat(&pos, "%c%c%c%c  %8s %8s",
                                  *(int *)replay->magic != 0 ? this->replayPaths[slot][15] : 'u',
                                  *(int *)replay->magic != 0 ? this->replayPaths[slot][16] : 's',
                                  *(int *)replay->magic != 0 ? this->replayPaths[slot][17] : 'e',
                                  *(int *)replay->magic != 0 ? this->replayPaths[slot][18] : 'r',
                                  *(int *)replay->magic != 0 ? replay->info.name : "--------",
                                  *(int *)replay->magic != 0 ? replay->info.date : "--/--/--");
        }
        pos.x = 221.0f;
        pos.y = 240.0f;
        if (this->replays[slot].mode == 0)
        {
            sprintf(buf, "%s  -%s-", g_ReplayMenuModeNames[0], g_ReplayMenuDifficultyNames[replay->info.difficulty]);
            pos.x = 320.0f - strlen(buf) * 4.5f;
            g_AsciiText.AddFormat(&pos, buf);
        }
        else if (this->replays[slot].mode == 1)
        {
            sprintf(buf, "%s", g_ReplayMenuModeNames[1]);
            pos.x = 320.0f - strlen(buf) * 4.5f;
            g_AsciiText.AddFormat(&pos, buf);
        }
        else if (this->replays[slot].mode == 2)
        {
            sprintf(buf, "%s  -%s-", g_ReplayMenuMatchNames[replay->unknown1e5],
                    g_ReplayMenuDifficultyNames[replay->info.difficulty]);
            pos.x = 320.0f - strlen(buf) * 4.5f;
            g_AsciiText.AddFormat(&pos, buf);
        }
        pos.x = 230.0f;
        pos.y = 270.0f;
        for (i = 0; i < 10; i++)
        {
            if (replay->unknown20[0][i] != NULL)
            {
                g_AsciiText.color = i == this->cursor ? 0xffffffff : 0xff808080;
                g_AsciiText.AddFormat(&pos, "%.8s vs %.8s",
                                      g_ReplayMenuCharacterNames[((ReplayRound *)replay->unknown20[0][i])->character],
                                      g_ReplayMenuCharacterNames[((ReplayRound *)replay->unknown20[1][i])->character]);
                pos.y += 13.0f;
            }
        }
    }
    g_AsciiText.color = 0xffffffff;
    g_AsciiText.highlight = 0;
    return 1;
}

int MainMenu::DrawState15()
{
    D3DXVECTOR3 pos(32.0f, 64.0f, 0.0f);
    int i;
    int slot;
    int row;
    int col;
    float scale;
    char str[16];

    if (this->subState == 2)
    {
        pos.y = 90.0f;
        for (i = 0; i < 25; i++)
        {
            g_AsciiText.color = i == this->cursor ? 0xffffffff : 0xff808080;
            g_AsciiText.AddFormat(&pos, "No.%.2d %.8s %8s", i + 1,
                                  *(int *)this->replays[i].magic != 0 ? this->replays[i].info.name : "--------",
                                  *(int *)this->replays[i].magic != 0 ? this->replays[i].info.date : "--/--/--");
            pos.y += 13.0f;
        }
    }
    else if (this->subState == 3 || this->subState == 4)
    {
        slot = this->replaySlot;
        pos.y = slot * 13.0f + 90.0f;
        D3DXVECTOR3 end = pos;
        end.x = end.y = 224.0f;
        if (this->unknown1b230 < 10)
        {
            pos = pos + (end - pos) * (float)this->unknown1b230 * 0.1f;
        }
        else
        {
            pos = end;
        }
        g_AsciiText.color = 0xffffffff;
        g_AsciiText.AddFormat(&pos, "No.%.2d %.8s %8s", slot + 1, this->replayName, "");
        pos.x += (this->replayNameCursor + 6) * 9;
        g_AsciiText.AddFormat(&pos, "%.8s", "_");
        if (this->subState == 4)
        {
            pos.z = 0.0f;
            pos.y = 320.0f;
            for (row = 0; row < 6; row++)
            {
                pos.x = 224.0f;
                for (col = 0; col < 16; col++)
                {
                    pos.x += 12.0f;
                    float offsetX = 0.0f;
                    float offsetY = 0.0f;
                    if (this->cursor == col + row * 16)
                    {
                        g_AsciiText.color = 0xffffffc0;
                        if (this->stateFrames % 64 < 32)
                        {
                            scale = this->stateFrames % 32 * 0.025f + 1.2f;
                        }
                        else
                        {
                            scale = 2.0f - this->stateFrames % 32 * 0.025f;
                        }
                        g_AsciiText.scaleX = scale;
                        g_AsciiText.scaleY = scale;
                        offsetY = offsetX = (scale - 1.0f) * -8.0f;
                    }
                    else
                    {
                        g_AsciiText.color = 0xc0c0c0c0;
                        g_AsciiText.scaleX = 1.0f;
                        g_AsciiText.scaleY = 1.0f;
                    }
                    D3DXVECTOR3 charPos = D3DXVECTOR3(offsetX, offsetY, 0.0f) + pos;
                    str[0] = g_NameAlphabet[row * 16 + col];
                    str[1] = 0;
                    if (row == 5)
                    {
                        if (col == 14)
                        {
                            str[0] = 0x7f;
                        }
                        else if (col == 15)
                        {
                            str[0] = 0x80;
                        }
                    }
                    g_AsciiText.AddString(&charPos, str);
                }
                pos.y += 16.0f;
            }
        }
    }
    g_AsciiText.color = 0xffffffff;
    g_AsciiText.highlight = 0;
    g_AsciiText.scaleX = 1.0f;
    g_AsciiText.scaleY = 1.0f;
    return 1;
}

int MainMenu::DrawState13()
{
    D3DXVECTOR3 pos;
    int row;
    int col;
    int mask;
    int highlight;
    int alpha;
    float scale;
    char str[16];
    Hscr (*scores)[5];

    scores = g_HighScores[this->scoreCharacter];
    if (this->state == 13)
    {
        mask = 0xff;
        highlight = 99;
    }
    else if (this->state == 14)
    {
        mask = 1 << g_GameManager.difficulty;
        highlight = this->scoreRank;
    }
    alpha = this->unknown1b230 < 30 ? this->unknown1b230 * 255 / 30 : 255;
    g_AsciiText.color = mask & 1 ? (alpha << 24) | 0xd0ffd0 : (alpha << 24) | 0x808080;
    pos.x = 102.0f;
    pos.y = 80.0f;
    pos.z = 0.0f;
    g_AsciiText.AddFormat(&pos, "Easy Ranking");
    pos.y += 16.0f;
    // Each table's index is a variable of its own, so that they take
    // the stack slots the original's do.
    {
        int i;

        for (i = 0; i < 5; i++)
        {
            if (mask & 1)
            {
                g_AsciiText.color = highlight == i ? (alpha << 24) | 0xfff0ef : (alpha << 24) | 0xc0f0ff;
            }
            else
            {
                g_AsciiText.color = (alpha << 24) | 0x809090;
            }
            if (mask & 1 && highlight == i)
            {
                pos.x += (this->replayNameCursor + 4) * 9;
                g_AsciiText.AddFormat(&pos, "%.8s", "_");
                pos.x -= (this->replayNameCursor + 4) * 9;
            }
            g_AsciiText.AddFormat(&pos, "%s %.8s %.9d%d", g_ScoreRankNames[i], (*scores)[i].name, (*scores)[i].score,
                                  (char)(*scores)[i].unknown2b);
            pos.y += 16.0f;
        }
    }
    scores++;
    g_AsciiText.color = mask & 2 ? (alpha << 24) | 0xd0ffd0 : (alpha << 24) | 0x808080;
    pos.x = 358.0f;
    pos.y = 80.0f;
    pos.z = 0.0f;
    g_AsciiText.AddFormat(&pos, "Normal Ranking");
    pos.y += 16.0f;
    {
        int i;
        for (i = 0; i < 5; i++)
        {
            if (mask & 2)
            {
                g_AsciiText.color = highlight == i ? (alpha << 24) | 0xfff0ef : (alpha << 24) | 0xc0f0ff;
            }
            else
            {
                g_AsciiText.color = (alpha << 24) | 0x809090;
            }
            if (mask & 2 && highlight == i)
            {
                pos.x += (this->replayNameCursor + 4) * 9;
                g_AsciiText.AddFormat(&pos, "%.8s", "_");
                pos.x -= (this->replayNameCursor + 4) * 9;
            }
            g_AsciiText.AddFormat(&pos, "%s %.8s %.9d%d", g_ScoreRankNames[i], (*scores)[i].name, (*scores)[i].score,
                                  (char)(*scores)[i].unknown2b);
            pos.y += 16.0f;
        }
    }
    scores++;
    g_AsciiText.color = mask & 4 ? (alpha << 24) | 0xfff0d0 : (alpha << 24) | 0x808080;
    pos.x = 102.0f;
    pos.y = 192.0f;
    pos.z = 0.0f;
    g_AsciiText.AddFormat(&pos, "Hard Ranking");
    pos.y += 16.0f;
    {
        int i;
        for (i = 0; i < 5; i++)
        {
            if (mask & 4)
            {
                g_AsciiText.color = highlight == i ? (alpha << 24) | 0xfff0ef : (alpha << 24) | 0xc0f0ff;
            }
            else
            {
                g_AsciiText.color = (alpha << 24) | 0x809090;
            }
            if (mask & 4 && highlight == i)
            {
                pos.x += (this->replayNameCursor + 4) * 9;
                g_AsciiText.AddFormat(&pos, "%.8s", "_");
                pos.x -= (this->replayNameCursor + 4) * 9;
            }
            g_AsciiText.AddFormat(&pos, "%s %.8s %.9d%d", g_ScoreRankNames[i], (*scores)[i].name, (*scores)[i].score,
                                  (char)(*scores)[i].unknown2b);
            pos.y += 16.0f;
        }
    }
    scores++;
    g_AsciiText.color = mask & 8 ? (alpha << 24) | 0xffc0c0 : (alpha << 24) | 0x808080;
    pos.x = 358.0f;
    pos.y = 192.0f;
    pos.z = 0.0f;
    g_AsciiText.AddFormat(&pos, "Lunatic Ranking");
    pos.y += 16.0f;
    {
        int i;
        for (i = 0; i < 5; i++)
        {
            if (mask & 8)
            {
                g_AsciiText.color = highlight == i ? (alpha << 24) | 0xfff0ef : (alpha << 24) | 0xc0f0ff;
            }
            else
            {
                g_AsciiText.color = (alpha << 24) | 0x809090;
            }
            if (mask & 8 && highlight == i)
            {
                pos.x += (this->replayNameCursor + 4) * 9;
                g_AsciiText.AddFormat(&pos, "%.8s", "_");
                pos.x -= (this->replayNameCursor + 4) * 9;
            }
            g_AsciiText.AddFormat(&pos, "%s %.8s %.9d%d", g_ScoreRankNames[i], (*scores)[i].name, (*scores)[i].score,
                                  (char)(*scores)[i].unknown2b);
            pos.y += 16.0f;
        }
    }
    scores++;
    g_AsciiText.color = mask & 16 ? (alpha << 24) | 0xffc0f0 : (alpha << 24) | 0x808080;
    pos.x = 102.0f;
    pos.y = 304.0f;
    pos.z = 0.0f;
    g_AsciiText.AddFormat(&pos, "Extra Ranking");
    pos.y += 16.0f;
    {
        int i;
        for (i = 0; i < 5; i++)
        {
            if (mask & 16)
            {
                g_AsciiText.color = highlight == i ? (alpha << 24) | 0xfff0ef : (alpha << 24) | 0xc0f0ff;
            }
            else
            {
                g_AsciiText.color = (alpha << 24) | 0x809090;
            }
            if (mask & 16 && highlight == i)
            {
                pos.x += (this->replayNameCursor + 4) * 9;
                g_AsciiText.AddFormat(&pos, "%.8s", "_");
                pos.x -= (this->replayNameCursor + 4) * 9;
            }
            g_AsciiText.AddFormat(&pos, "%s %.8s %.9d%d", g_ScoreRankNames[i], (*scores)[i].name, (*scores)[i].score,
                                  (char)(*scores)[i].unknown2b);
            pos.y += 16.0f;
        }
    }
    if (this->state == 13)
    {
    }
    else if (this->state == 14 && this->scoreRank < 5 && this->subState >= 1)
    {
        pos.z = 0.0f;
        pos.y = 336.0f;
        for (row = 0; row < 6; row++)
        {
            pos.x = 348.0f;
            for (col = 0; col < 16; col++)
            {
                pos.x += 12.0f;
                float offsetX = 0.0f;
                float offsetY = 0.0f;
                if (this->cursor == col + row * 16)
                {
                    g_AsciiText.color = 0xffffffc0;
                    if (this->stateFrames % 64 < 32)
                    {
                        scale = this->stateFrames % 32 * 0.025f + 1.2f;
                    }
                    else
                    {
                        scale = 2.0f - this->stateFrames % 32 * 0.025f;
                    }
                    g_AsciiText.scaleX = scale;
                    g_AsciiText.scaleY = scale;
                    offsetY = offsetX = (scale - 1.0f) * -8.0f;
                }
                else
                {
                    g_AsciiText.color = 0xc0c0c0c0;
                    g_AsciiText.scaleX = 1.0f;
                    g_AsciiText.scaleY = 1.0f;
                }
                D3DXVECTOR3 charPos = D3DXVECTOR3(offsetX, offsetY, 0.0f) + pos;
                str[0] = g_NameAlphabet[row * 16 + col];
                str[1] = 0;
                if (row == 5)
                {
                    if (col == 14)
                    {
                        str[0] = 0x7f;
                    }
                    else if (col == 15)
                    {
                        str[0] = 0x80;
                    }
                }
                g_AsciiText.AddString(&charPos, str);
            }
            pos.y += 16.0f;
        }
    }
    g_AsciiText.color = 0xffffffff;
    g_AsciiText.scaleX = 1.0f;
    g_AsciiText.scaleY = 1.0f;
    return 1;
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

int MainMenu::OnDraw(MainMenu *menu)
{
    int i;
    Sprite *vm;

    g_App.SetView(2);
    if (menu->loadingState != 0)
    {
        return 1;
    }
    g_SpriteEngine->currentTexture = NULL;
    g_SpriteEngine->CopySurfaceToBackBuffer(0, 0, 0, 0, 0);
    vm = menu->sprites;
    for (i = 0; i < menu->spriteCount; i++, vm++)
    {
        if (g_SpriteEngine->HasTexture(vm))
        {
            D3DXVECTOR3 savedPos = vm->pos;
            vm->pos += vm->posOffset;
            if (vm->rotation.z != 0.0f)
            {
                g_SpriteEngine->Draw(vm);
            }
            else
            {
                g_SpriteEngine->DrawUnrotated(vm);
            }
            vm->pos = savedPos;
        }
    }
    if (menu->helpSprite != NULL)
    {
        g_SpriteEngine->DrawUnrotated(menu->helpSprite);
    }
    switch (menu->state)
    {
    case 3:
        menu->DrawNothing();
        break;
    case 5:
        menu->DrawNothing();
        break;
    case 7:
        menu->DrawNothing();
        break;
    case 11:
        menu->DrawState11();
        break;
    case 12:
        menu->DrawState12();
        break;
    case 13:
    case 14:
        menu->DrawState13();
        break;
    case 15:
        menu->DrawState15();
        break;
    }
    g_SpriteEngine->FlushQuadBatch();
    return 1;
}

void MainMenu::LoadingThread(MainMenu *menu)
{
    int i;

    while (g_SpriteEngine->pendingCopyTexture >= 0)
    {
        Sleep(1);
    }
    g_MainMenu->anm = g_SpriteEngine->PreloadAnm(0xf, "title01.anm");
    if (g_MainMenu->anm == NULL)
    {
        g_MainMenu->loadingState = 2;
        return;
    }
    g_MainMenu->resultTextAnm = g_SpriteEngine->PreloadAnm(0x11, "resulttext.anm");
    if (g_MainMenu->resultTextAnm == NULL)
    {
        g_MainMenu->loadingState = 2;
        return;
    }
    if (g_App.closeSubThread != 0)
    {
        return;
    }
    for (i = 0; i < 14; i++)
    {
        g_App.textAnm->StartScript(&g_MainMenu->helpSprites[i], 5);
        g_App.textAnm->SetSprite(&g_MainMenu->helpSprites[i], g_MainMenu->helpSprites[i].spriteIndex + i);
        g_App.textAnm->SetSprite(&g_MainMenu->cardDetailSprites[i], i + 0x15);
        if (i <= 4)
        {
            g_MainMenu->cardDetailSprites[i].pos = D3DXVECTOR3(64.0f, i * 16.0f + 352.0f, 0.0f);
        }
        else
        {
            g_MainMenu->cardDetailSprites[i].pos = D3DXVECTOR3(64.0f, i * 16.0f + 362.0f, 0.0f);
        }
        g_MainMenu->cardDetailSprites[i].flags |= 0x1800;
        g_MainMenu->cardDetailSprites[i].fontWidth = 15;
        g_MainMenu->cardDetailSprites[i].fontHeight = 15;
        g_MainMenu->cardDetailSprites[i].flags |= 2;
    }
    if (g_App.closeSubThread != 0)
    {
        return;
    }
    if (g_MainMenu->state == 1)
    {
        if (g_SpriteEngine->PreloadSurface(0, "title/title00.png") != 0)
        {
            g_MainMenu->loadingState = 2;
            return;
        }
    }
    else if (g_SpriteEngine->PreloadSurface(0, "title/result00.png") != 0)
    {
        g_MainMenu->loadingState = 2;
        return;
    }
    if (!(g_GameManager.stateFlags & 2))
    {
        if (g_App.prevScene != 5)
        {
            g_App.LoadBgm(0);
        }
        if (g_App.lastPlayTimeTick == 0)
        {
            ScreenEffect::StartEffect(0, 70, 0xffffff, 0, 0, 0x23, 2);
        }
        else
        {
            ScreenEffect::StartEffect(0, 70, 0xffffff, 0, 0, 0x23, 2);
        }
    }
    g_MainMenu->helpSprite = g_MainMenu->helpSprites;
    g_MainMenu->loadingState = 0;
    if (g_Unknown4a7c80 != NULL)
    {
        g_Unknown4a7c80->Finish();
    }
    g_Unknown4a7c80 = NULL;
    g_App.StopLoadingSprites();
    g_App.subThread = NULL;
    g_App.closeSubThread = 0;
    g_App.unknown6b8 = 0;
}


int MainMenu::ReleaseTitleScene()
{
    if (this->unknownC8fc != NULL)
    {
        free(this->unknownC8fc);
        this->unknownC8fc = NULL;
    }
    if (this->sprites != NULL)
    {
        delete[] this->sprites;
        this->sprites = NULL;
    }
    for (int i = 0; i < 50; i++)
    {
        if (this->unknown681c[i] != NULL)
        {
            free(this->unknown681c[i]);
        }
        this->unknown681c[i] = NULL;
    }
    return 0;
}

int MainMenu::DeletedCallback(MainMenu *menu)
{
    g_App.d3dDevice->ResourceManagerDiscardBytes(0);
    g_SpriteEngine->ReleaseAnm(0xf);
    g_SpriteEngine->ReleaseAnm(0x11);
    g_SpriteEngine->ReleaseSurface(0);
    g_Chain.Remove(menu->drawChain);
    menu->drawChain = NULL;
    menu->ReleaseTitleScene();
    delete menu;
    return 0;
}

int MainMenu::InitTitleScene()
{
    g_Unknown4ac884 = 0;
    g_AsciiText.Initialize();
    g_AsciiText.StartScripts();
    g_App.gameSpeed = 1.0f;
    if (g_GameManager.stateFlags & 8)
    {
        g_GameManager.sides[0].character = 0;
        g_GameManager.sides[1].character = 1;
    }
    if (g_GameManager.stateFlags & 2)
    {
        g_GameManager.stateFlags &= ~8;
    }
    this->SetState(1);
    g_App.ClearUnknown77c();
    if (g_Network->unknownA8 != 0 && g_GameManager.mode != 2)
    {
        this->unknown1b23c = 2;
        g_GameManager.unknown114 = 0;
        g_App.config.difficulty = 1;
    }
    switch (this->unknown1b22c)
    {
    case 0:
        if (g_GameManager.stateFlags & 0xa)
        {
            break;
        }
        if (g_GameManager.mode == 0)
        {
            this->cursor = 0;
        }
        else if (g_GameManager.mode == 1)
        {
            this->cursor = 1;
        }
        else if (g_GameManager.mode == 2)
        {
            this->unknown1b23c = 1;
        }
        break;
    case 1:
        this->state = 14;
        break;
    default:
        this->cursor = 0;
        break;
    }
    this->unknown88 = 0;
    g_GameManager.stateFlags &= ~1;
    D3DXVECTOR3 pos(500.0f, 440.0f, 0.0f);
    if (g_App.prevScene == 2)
    {
        g_App.StartLoadingSpritesOverScreen(&pos);
        g_App.StartLoadingScreenEffect(0);
    }
    else if (g_App.prevScene != 0)
    {
        g_App.StartLoadingSprites(&pos);
    }
    g_GameManager.stateFlags &= ~2;
    this->loadingState = 1;
    g_App.StartSubThread((LPTHREAD_START_ROUTINE)LoadingThread, NULL);
    return 0;
}

int MainMenu::AddedCallback(MainMenu *menu)
{
    return menu->InitTitleScene();
}

void MainMenu::SelectEntry(int cursor, int first, int count)
{
    for (int i = first; i < first + count; i++)
    {
        this->anm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        this->sprites[i].pendingInterrupt = 8;
    }
    this->anm->SetSprite(&this->sprites[cursor + first], this->sprites[cursor + first].baseSpriteIndex);
    this->sprites[cursor + first].pendingInterrupt = 7;
}

void MainMenu::ShowEntryFrame(int frame, int idx)
{
    this->anm->SetSprite(&this->sprites[idx], this->sprites[idx].baseSpriteIndex + frame * 2);
    this->sprites[idx].pendingInterrupt = 7;
}

void MainMenu::SelectEntryStep(int cursor, int first, int count, int stride)
{
    for (int i = first; i < first + count * stride; i += stride)
    {
        this->anm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        this->sprites[i].pendingInterrupt = 8;
        g_SpriteEngine->Step(&this->sprites[i]);
    }
    this->anm->SetSprite(&this->sprites[cursor * stride + first], this->sprites[cursor * stride + first].baseSpriteIndex);
    this->sprites[cursor * stride + first].pendingInterrupt = 7;
    g_SpriteEngine->Step(&this->sprites[cursor * stride + first]);
}

void MainMenu::SelectEntry2(int cursor, int first, int count, int stride)
{
    for (int i = first; i < first + count * stride; i += stride)
    {
        this->anm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        this->sprites[i].pendingInterrupt = 15;
    }
    this->anm->SetSprite(&this->sprites[cursor * stride + first], this->sprites[cursor * stride + first].baseSpriteIndex);
    this->sprites[cursor * stride + first].pendingInterrupt = 14;
}

void MainMenu::SelectEntry3(int cursor, int first, int count, int stride)
{
    for (int i = first; i < first + count * stride; i += stride)
    {
        this->anm->SetSprite(&this->sprites[i], this->sprites[i].baseSpriteIndex + 1);
        this->sprites[i].pendingInterrupt = 18;
    }
    this->anm->SetSprite(&this->sprites[cursor * stride + first], this->sprites[cursor * stride + first].baseSpriteIndex);
    this->sprites[cursor * stride + first].pendingInterrupt = 17;
}

void MainMenu::SelectEntryInterrupts(int cursor, int first, int count)
{
    for (int i = first; i < first + count; i++)
    {
        this->sprites[i].pendingInterrupt = 8;
    }
    this->sprites[cursor + first].pendingInterrupt = 7;
}

void MainMenu::FlashHalves(int left, int right)
{
    this->sprites[141].pendingInterrupt = 9;
    this->sprites[147].pendingInterrupt = 9;
    this->sprites[142].pendingInterrupt = 9;
    this->sprites[143].pendingInterrupt = 9;
    this->sprites[144].pendingInterrupt = 9;
    this->sprites[145].pendingInterrupt = 9;
    this->sprites[146].pendingInterrupt = 9;
    this->sprites[148].pendingInterrupt = 9;
    this->sprites[149].pendingInterrupt = 9;
    this->sprites[150].pendingInterrupt = 9;
    this->sprites[151].pendingInterrupt = 9;
    this->sprites[152].pendingInterrupt = 9;
    this->ShowHalves(left, right);
}

void MainMenu::FlashSprite(int idx, int on)
{
    this->sprites[idx + 153].pendingInterrupt = on ? 9 : 10;
}

void MainMenu::MoveCharacterCursor(int player, int selected, int selected2, char *entries)
{
    Sprite *vm = &this->sprites[player + 0xbd];
    int row = 0;
    int i;
    for (i = 0; i < 16; i++)
    {
        if (selected == entries[i])
        {
            break;
        }
        if (this->state == 8)
        {
            if (g_PlayRecord.openEntries[entries[i]] || g_Network->unknownA8 != 0)
            {
                row++;
            }
        }
        else if (this->state == 3)
        {
            if (g_PlayRecord.openEntries2[entries[i]])
            {
                row++;
            }
        }
        else if (g_PlayRecord.openEntries3[entries[i]])
        {
            row++;
        }
    }
    D3DXVECTOR3 to;
    to.x = vm->pos.x;
    to.y = row * 16.0f + 128.0f;
    to.z = 0.0f;
    vm->SetMoveInterp(8, 4, &vm->pos, &to);
    int row2 = 0;
    for (i = 0; i < 16; i++)
    {
        if (selected2 == entries[i])
        {
            break;
        }
        if (this->state == 8)
        {
            if (g_PlayRecord.openEntries[entries[i]] || g_Network->unknownA8 != 0)
            {
                row2++;
            }
        }
        else if (this->state == 3)
        {
            if (g_PlayRecord.openEntries2[entries[i]])
            {
                row2++;
            }
        }
        else if (g_PlayRecord.openEntries3[entries[i]])
        {
            row2++;
        }
    }
    int j = 0;
    for (i = 0; i < 16; i++)
    {
        if (g_PlayRecord.openEntries[entries[i]] || g_Network->unknownA8 != 0)
        {
            vm = &this->sprites[j + 191];
            if (row == j || row2 == j)
            {
                vm->SetColorInterp(8, 4, vm->color, 0xffffffff);
            }
            else
            {
                vm->SetColorInterp(8, 4, vm->color, 0xffa0a0a0);
            }
            j++;
        }
    }
}

void MainMenu::MoveCharacterCursor2(int player, int selected, char *entries)
{
    int row = 0;
    Sprite *vm = &this->sprites[189];
    int i;
    for (i = 0; i < 16; i++)
    {
        if (selected == entries[i])
        {
            break;
        }
        if (i > 13 || g_PlayRecord.openEntries[entries[i]] || g_Network->unknownA8 != 0)
        {
            row++;
        }
    }
    D3DXVECTOR3 to;
    to.x = vm->pos.x;
    to.y = row * 16.0f + 128.0f;
    to.z = 0.0f;
    vm->SetMoveInterp(8, 4, &vm->pos, &to);
    vm = &this->sprites[190];
    to.x = vm->pos.x;
    vm->SetMoveInterp(8, 4, &vm->pos, &to);
    int j = 0;
    for (i = 0; i < 16; i++)
    {
        if (g_PlayRecord.openEntries[entries[i]] || i > 13 || g_Network->unknownA8 != 0)
        {
            vm = &this->sprites[j + 207];
            if (row == j)
            {
                vm->SetColorInterp(8, 4, vm->color, 0xffffffff);
            }
            else
            {
                vm->SetColorInterp(8, 4, vm->color, 0xffa0a0a0);
            }
            j++;
        }
    }
}

void MainMenu::SelectCursorEntry()
{
    this->SelectEntry(this->cursor, this->entryFirst, this->entryCount);
}

MainMenu::MainMenu()
{
    memset(this, 0, sizeof(MainMenu));
}

int MainMenu::OnUpdate(MainMenu *menu)
{
    int ret;

    ret = 1;
    if (menu->loadingState != 0)
    {
        return menu->loadingState == 2 ? 4 : 1;
    }
    switch (menu->state)
    {
    case 1:
        ret = menu->UpdateState1();
        break;
    case 6:
        ret = menu->UpdateState6();
        break;
    case 2:
    case 4:
    case 7:
        ret = menu->UpdateState2();
        break;
    case 3:
    case 5:
        ret = menu->UpdateState3();
        break;
    case 8:
        switch (g_GameManager.unknown114)
        {
        case 0:
            ret = menu->UpdateState8();
            break;
        case 1:
            ret = menu->UpdateState8b();
            break;
        case 2:
            ret = menu->UpdateState8b();
            break;
        case 3:
            ret = menu->UpdateState8b();
            break;
        case 4:
            ret = menu->UpdateState8();
            break;
        }
        break;
    case 16:
        ret = menu->UpdateState16();
        break;
    case 9:
        ret = menu->UpdateState9();
        break;
    case 10:
        ret = menu->UpdateState10();
        break;
    case 11:
        ret = menu->UpdateState11();
        break;
    case 15:
        ret = menu->UpdateState15();
        break;
    case 12:
        ret = menu->UpdateState12();
        break;
    case 14:
        ret = menu->UpdateState14();
        break;
    case 13:
        ret = menu->UpdateState13();
        break;
    }
    g_SpriteEngine->StepSprites(menu->sprites, menu->spriteCount);
    if (menu->helpSprite != NULL)
    {
        g_SpriteEngine->Step(menu->helpSprite);
    }
    return ret;
}

int MainMenu::UpdateState1()
{
    int delta;

    switch (this->subState)
    {
    case 0:
        if (this->unknown1b230 == 0)
        {
            if (this->prevState == 0 || this->prevState == 14 || this->prevState == 12 || this->prevState == 15)
            {
                g_App.PlayMusic(0, 0);
            }
            if ((this->prevState == 0 || this->prevState == 13 || this->prevState == 14 || this->prevState == 11 ||
                 this->prevState == 15 || this->prevState == 12 || this->prevState == 2 || this->prevState == 4 ||
                 this->prevState == 7 || this->prevState == 6) &&
                g_SpriteEngine->LoadSurface(0, "title/title00.png") != 0)
            {
                return 0;
            }
            ReplayManager::Release(g_GameManager.replayManager);
            g_GameManager.replayManager = NULL;
            if (this->spriteCount == 0)
            {
                this->spriteCount = 0xdf;
                this->sprites = new Sprite[this->spriteCount];
                this->anm->StartSpriteSeries(this->sprites, 0, this->spriteCount);
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 2);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            if (!g_PlayRecord.openEntries3[12])
            {
                this->sprites[2].color = 0x30908040;
            }
            if (this->unknown1b23c != 0)
            {
                g_GameManager.stateFlags &= ~8;
                this->cursor = g_GameManager.unknown114;
                this->SetState(6);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                return 1;
            }
            this->entryFirst = 1;
            this->entryCount = 8;
            this->SelectCursorEntry();
            if (g_GameManager.stateFlags & 8)
            {
                this->SetState(11);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                return 1;
            }
            g_GameManager.stateFlags &= ~2;
        }
        if (this->unknown1b230 < ARRAY_SIZE(g_TitleHelpTexts))
        {
            g_SpriteEngine->RenderTextCentered(&this->helpSprites[this->unknown1b230], 0xfff0e0, 0x300000,
                                               g_TitleHelpTexts[this->unknown1b230]);
            this->unknown1b230++;
            return 1;
        }
        this->prevCursor = -1;
        this->unknown1b230 = 0;
        this->stateFrames = 0;
        this->subState = 1;
        this->idleFrames = 0;
    case 1:
        delta = this->MoveCursor(8);
        if (delta != 0)
        {
            if (!g_PlayRecord.openEntries3[12] && this->cursor == 1)
            {
                this->cursor += delta;
            }
            this->SelectCursorEntry();
        }
        if (this->prevCursor != this->cursor)
        {
            this->helpSprite = &this->helpSprites[this->cursor];
            this->helpSprite->SetPendingInterrupt(1);
        }
        this->prevCursor = this->cursor;
        if (!g_PlayRecord.openEntries3[12])
        {
            this->sprites[2].color = 0xb0b0b040;
        }
        if (this->unknown1b230 < 10)
        {
            break;
        }
        if (g_PlayerInputs[2].unknown0 != 0)
        {
            this->idleFrames = 0;
        }
        this->idleFrames++;
        if (this->idleFrames > 1500)
        {
            g_GameManager.stateFlags |= 2;
            strcpy(g_GameManager.replayFile, g_DemoReplayPaths[g_GameManager.demoIndex]);
            g_GameManager.demoIndex++;
            g_GameManager.demoIndex %= 3;
            g_GameManager.stateFlags |= 8;
            g_GameManager.unknownFc = 9;
            g_GameManager.difficulty = 3;
            g_GameManager.sides[0].unknown28 = 0;
            g_GameManager.sides[1].unknown28 = 1;
            g_App.scene = 2;
            g_GameManager.mode = 2;
            return 0;
        }
        if (g_PlayerInputs[2].unknown6 & 0x1001)
        {
            switch (this->cursor)
            {
            case 0:
                g_GameManager.stateFlags &= ~8;
                this->PlaySoundNow(10, 0);
                if (this->cursor >= 4)
                {
                    this->cursor = 2;
                }
                this->SetState(2);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                g_GameManager.unknown378 = 10;
                g_GameManager.unknown37c = 10;
                return 1;
            case 1:
                if (g_PlayRecord.openEntries3[12])
                {
                    g_GameManager.stateFlags &= ~8;
                    this->PlaySoundNow(10, 0);
                    if (this->cursor >= 4)
                    {
                        this->cursor = 2;
                    }
                    this->SetState(4);
                    g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                    g_GameManager.unknown378 = 10;
                    g_GameManager.unknown37c = 10;
                    return 1;
                }
                this->PlaySoundNow(0x27, 0);
                return 1;
            case 2:
                g_GameManager.stateFlags &= ~8;
                this->PlaySoundNow(10, 0);
                this->cursor = g_GameManager.unknown114;
                this->SetState(6);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                g_GameManager.unknown378 = 10;
                g_GameManager.unknown37c = 10;
                return 1;
            case 3:
                this->PlaySoundNow(10, 0);
                this->SetState(11);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                this->helpSprite->SetPendingInterrupt(2);
                return 1;
            case 5:
                this->PlaySoundNow(10, 0);
                this->SetState(12);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                this->helpSprite->SetPendingInterrupt(2);
                return 1;
            case 4:
                this->PlaySoundNow(10, 0);
                this->SetState(13);
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 5);
                return 1;
            case 6:
                this->PlaySoundNow(10, 0);
                this->subState = 0;
                this->cursor = 0;
                this->unknown1b230 = 0;
                this->stateFrames = 0;
                this->UpdateState9();
                this->SelectEntry(g_App.config.unknownAc, 0x12, 5);
                this->SelectEntry(g_App.config.colorMode16Bits, 0x15, 2);
                this->SelectEntry(g_App.config.musicMode, 0x17, 3);
                this->SelectEntry(g_App.config.windowed, 0x22, 2);
                this->subState = 3;
                this->stateFrames = 0;
                this->unknownC920 = 0;
                this->cursor = 0;
                break;
            case 7:
                this->subState = 2;
                this->stateFrames = 0;
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 1);
                if (g_App.config.musicMode == 2)
                {
                    g_App.midi->Restart(30);
                }
                break;
            }
        }
        if (g_PlayerInputs[2].unknown6 & 10)
        {
            this->anm->SetSprite(&this->sprites[this->cursor + 1], this->sprites[this->cursor + 1].baseSpriteIndex + 1);
            this->cursor = 7;
            this->anm->SetSprite(&this->sprites[this->cursor + 1], this->sprites[this->cursor + 1].baseSpriteIndex);
            this->PlaySoundNow(11, 0);
        }
        break;
    case 2:
        if (this->stateFrames >= 60)
        {
            delete[] this->sprites;
            this->sprites = NULL;
            this->spriteCount = 0;
            this->unknown1b230 = 0;
            g_App.scene = -1;
            return 0;
        }
        break;
    case 3:
        if (this->stateFrames >= 30)
        {
            this->SetState(9);
            this->cursor = 0;
            this->config = g_App.config;
            return 1;
        }
    }
    this->unknownC910++;
    this->stateFrames++;
    this->unknown1b230++;
    return 1;
}

int MainMenu::UpdateState13()
{
    int i;
    int delta;
    char character;

    switch (this->subState)
    {
    case 0:
        if (this->unknown1b230 == 0)
        {
            if (g_SpriteEngine->LoadSurface(0, "title/result00.png") != 0)
            {
                return 0;
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 16);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            this->cursor = 0;
            this->scoreCharacter = 0;
            this->SelectEntry3(g_ScoreCharacters[0], 0x3c, 0x10, 2);
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
            this->scoreRank = 0;
        }
        if (this->unknown1b230 == 8)
        {
            this->subState = 1;
        }
        break;
    case 1:
        if (g_CheatCodeProgress < 3)
        {
            if (g_PlayerInputs[2].unknown6 & 0x2000)
            {
                g_CheatCodeProgress++;
            }
            else if (g_PlayerInputs[2].unknown6)
            {
                g_CheatCodeProgress = 0;
            }
        }
        else if (g_CheatCodeProgress < 7)
        {
            if (g_PlayerInputs[2].unknown6 & 0x400)
            {
                g_CheatCodeProgress++;
            }
            else if (g_PlayerInputs[2].unknown6)
            {
                g_CheatCodeProgress = 0;
            }
        }
        else if (g_CheatCodeProgress < 8)
        {
            if (g_PlayerInputs[2].unknown6 & 0x200)
            {
                g_CheatCodeProgress++;
            }
            else if (g_PlayerInputs[2].unknown6)
            {
                g_CheatCodeProgress = 0;
            }
        }
        else if (g_CheatCodeProgress < 10)
        {
            if (g_PlayerInputs[2].unknown6 & 0x4000)
            {
                g_CheatCodeProgress++;
            }
            else if (g_PlayerInputs[2].unknown6)
            {
                g_CheatCodeProgress = 0;
            }
        }
        if (g_CheatCodeProgress >= 10)
        {
            // The (char) keeps the stores a loop, as the original's.
            for (i = 0; i < 16; i++)
            {
                g_PlayRecord.openEntries[(char)i] = 1;
            }
            for (i = 0; i <= 13; i++)
            {
                g_PlayRecord.openEntries2[(char)i] = 1;
                g_PlayRecord.openEntries3[(char)i] = 1;
            }
            g_SoundPlayer.PlaySoundByIdx(0x1c, 0);
            g_CheatCodeProgress = 0;
        }
        if ((delta = this->MoveCursorLR(14)) != 0)
        {
            while (!g_PlayRecord.openEntries2[this->cursor])
            {
                if (g_PlayRecord.openEntries3[this->cursor])
                {
                    break;
                }
                this->cursor += delta;
                if (this->cursor < 0)
                {
                    this->cursor += 16;
                }
                if (this->cursor >= 16)
                {
                    this->cursor -= 16;
                }
            }
            character = g_ScoreCharacters[this->cursor];
            this->SelectEntry3(character, 0x3c, 0x10, 2);
            this->scoreCharacter = character;
        }
        if (g_PlayerInputs[2].unknown6 & 10)
        {
            this->PlaySoundNow(11, 0);
            this->stateFrames = 0;
            this->SetState(1);
            this->cursor = 4;
            return 1;
        }
        break;
    }
    this->stateFrames++;
    this->unknownC910++;
    this->unknown1b230++;
    return 1;
}

int MainMenu::UpdateState2()
{
    switch (this->subState)
    {
    case 0:
        if (this->unknown1b230 == 0)
        {
            if (g_SpriteEngine->LoadSurface(0, "title/select00.png") != 0)
            {
                return 0;
            }
            if (this->state == 4)
            {
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 25);
            }
            else
            {
                g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 12);
            }
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            if (this->state == 7)
            {
                this->sprites[0x83 + g_GameManager.unknown114].pendingInterrupt = 9;
                if (this->unknown1b23c == 1)
                {
                    this->SetState(8);
                    switch (g_GameManager.unknown114)
                    {
                    case 0:
                        this->UpdateState8();
                        break;
                    case 1:
                        this->UpdateState8b();
                        break;
                    case 2:
                        this->UpdateState8b();
                        break;
                    case 3:
                        this->UpdateState8b();
                        break;
                    }
                    this->sprites[0x88 + g_App.config.difficulty].pendingInterrupt = 10;
                    this->unknown1b23c = 0;
                    return 1;
                }
                if (this->unknown1b23c == 2)
                {
                    this->UpdateState8();
                    this->sprites[0x88 + g_App.config.difficulty].pendingInterrupt = 10;
                    this->unknown1b23c = 0;
                    this->SetState(8);
                    return 1;
                }
            }
            this->cursor = g_App.config.difficulty;
            if (this->state == 4)
            {
                this->cursor = 4;
            }
            else if (this->cursor >= 4)
            {
                this->cursor = 1;
            }
            this->entryFirst = 0x88;
            this->entryCount = 4;
            this->SelectCursorEntry();
            this->unknownC918 = 1;
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
        }
        if (this->unknown1b230 == 8)
        {
            this->subState = 1;
        }
        break;
    case 1:
        if (this->state != 4)
        {
            if (this->MoveCursor(4) != 0)
            {
                this->SelectCursorEntry();
            }
        }
        else if (g_PlayerInputs[2].unknown6 & 0x30)
        {
            this->PlaySoundNow(12, 0);
        }
        if (g_PlayerInputs[2].unknown6 & 0x1001)
        {
            g_App.config.difficulty = this->cursor;
            this->PlaySoundNow(10, 0);
            if (this->state == 7)
            {
                this->SetState(8);
                switch (g_GameManager.unknown114)
                {
                case 0:
                    this->UpdateState8();
                    break;
                case 1:
                    this->UpdateState8b();
                    break;
                case 2:
                    this->UpdateState8b();
                    break;
                case 3:
                    this->UpdateState8b();
                    break;
                case 4:
                    this->UpdateState8();
                    break;
                }
                this->sprites[0x88 + g_App.config.difficulty].pendingInterrupt = 10;
                return 1;
            }
            if (this->state == 2)
            {
                this->SetState(3);
            }
            else
            {
                this->SetState(5);
            }
            this->UpdateState3();
            this->sprites[0x88 + g_App.config.difficulty].pendingInterrupt = 9;
            return 1;
        }
        if (g_GameManager.unknown114 == 4 && g_Network->unknownA8 == 0)
        {
            this->PlaySoundNow(11, 0);
            this->stateFrames = 0;
            if (this->state == 7)
            {
                this->SetState(6);
                return 1;
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 6);
            this->subState = 3;
        }
        if (g_Network->unknownA8 == 0 && (g_PlayerInputs[2].unknown6 & 0xa))
        {
            g_App.config.difficulty = this->cursor;
            this->PlaySoundNow(11, 0);
            this->stateFrames = 0;
            if (this->state == 7)
            {
                this->SetState(6);
                return 1;
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 6);
            this->subState = 3;
        }
        break;
    case 3:
        if (this->stateFrames >= 20)
        {
            int state = this->state;
            this->SetState(1);
            this->cursor = state != 2;
            return 1;
        }
        break;
    }
    this->stateFrames++;
    this->unknownC910++;
    this->unknown1b230++;
    return 1;
}

int MainMenu::UpdateState6()
{
    switch (this->subState)
    {
    case 0:
        if (this->unknown1b230 == 0)
        {
            if (this->prevState != 7 && g_SpriteEngine->LoadSurface(0, "title/select00.png") != 0)
            {
                return 0;
            }
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 11);
            g_SpriteEngine->StepSprites(this->sprites, this->spriteCount);
            if (this->unknown1b23c != 0)
            {
                this->SetState(7);
                this->UpdateState2();
                this->sprites[0x83 + g_GameManager.unknown114].pendingInterrupt = 9;
                return 1;
            }
            this->cursor = g_GameManager.unknown114;
            this->entryFirst = 0x83;
            this->entryCount = 5;
            this->SelectCursorEntry();
            this->unknownC918 = 1;
            this->subState = 0;
            this->stateFrames = 0;
            this->helpSprite = NULL;
        }
        if (this->unknown1b230 == 8)
        {
            this->subState = 1;
        }
        break;
    case 1:
        if (this->MoveCursor(5) != 0)
        {
            this->SelectCursorEntry();
        }
        if (g_PlayerInputs[2].unknown6 & 0x1001)
        {
            g_GameManager.unknown114 = this->cursor;
            if (g_GameManager.unknown114 == 4)
            {
                g_Network->Init();
                g_App.ResetRun();
                g_App.config.difficulty = 1;
                g_GameManager.sides[0].character = 0;
                g_GameManager.sides[1].character = 1;
            }
            this->PlaySoundNow(10, 0);
            this->SetState(7);
            this->UpdateState2();
            this->sprites[0x83 + g_GameManager.unknown114].pendingInterrupt = 9;
            return 1;
        }
        if (g_PlayerInputs[2].unknown6 & 0xa)
        {
            g_GameManager.unknown114 = this->cursor;
            this->PlaySoundNow(11, 0);
            this->subState = 3;
            this->stateFrames = 0;
            g_SpriteEngine->SendInterruptToLoadedSprites(this->sprites, this->spriteCount, 6);
        }
        break;
    case 3:
        if (this->stateFrames >= 20)
        {
            this->SetState(1);
            this->cursor = 2;
            return 1;
        }
        break;
    }
    this->stateFrames++;
    this->unknownC910++;
    this->unknown1b230++;
    return 1;
}

int SetUpTitleScene(int arg)
{
    MainMenu *menu;

    menu = new MainMenu;
    g_MainMenu = menu;
    memset(menu, 0, sizeof(MainMenu));
    g_GameManager.isPaused = 0;
    menu->unknown1b22c = arg;
    menu->calcChain = g_Chain.CreateElem((ChainCallback)MainMenu::OnUpdate);
    menu->calcChain->arg = menu;
    menu->calcChain->addedCallback = (ChainCallback)MainMenu::AddedCallback;
    menu->calcChain->deletedCallback = (ChainCallback)MainMenu::DeletedCallback;
    if (g_Chain.AddToCalcChain(menu->calcChain, 4) != 0)
    {
        return -1;
    }
    menu->drawChain = g_Chain.CreateElem((ChainCallback)MainMenu::OnDraw);
    menu->drawChain->arg = menu;
    g_Chain.AddToDrawChain(menu->drawChain, 3);
    return 0;
}
