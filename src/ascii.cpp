// The ASCII text, from 0x434280, as th08's ascii.cpp.

#include <stdio.h>
#include <string.h>

#include "ascii.h"
#include "gamemanager.h"
#include "player.h"
#include "unsorted.h"

int AsciiText::DeletedCallback(AsciiText *text)
{
    g_SpriteEngine->ReleaseAnm(1);
    g_SpriteEngine->ReleaseAnm(3);
    return 0;
}

void AsciiText::RemoveChain()
{
    g_Chain.Remove(&g_AsciiCalcChain);
    g_Chain.Remove(&g_AsciiTextElemDraw);
}

void AsciiText::AddString(D3DXVECTOR3 *pos, char *text)
{
    AsciiString *str;

    if (this->stringCount >= 0x100)
    {
        return;
    }
    str = &this->strings[this->stringCount];
    this->stringCount++;
    strcpy(str->text, text);
    str->pos = *pos;
    str->color = this->color;
    str->scaleX = this->scaleX;
    str->scaleY = this->scaleY;
    str->inArena = this->inArena;
    str->highlight = 0;
}

void AsciiText::AddFormat(D3DXVECTOR3 *pos, char *fmt, ...)
{
    char buf[0x200];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    this->AddString(pos, buf);
    va_end(args);
}

void AsciiText::Initialize()
{
    memset(&this->digit, 0, sizeof(this->digit));
    memset(&this->digit2, 0, sizeof(this->digit2));
    memset(&this->glyph, 0, sizeof(this->glyph));
    memset(this->strings, 0, sizeof(this->strings));
    memset(&this->pauseMenu, 0, sizeof(this->pauseMenu));
    memset(&this->continueMenu, 0, sizeof(this->continueMenu));
    memset(this->popups, 0, sizeof(this->popups[0]));
    this->stringCount = 0;
    this->inArena = 0;
    this->highlight = 0;
    this->nextPopup = 0;
    this->nextPopup2 = 0;
    this->unknown829c = 0;
    this->color = 0xffffffff;
    this->scaleX = 1.0f;
    this->scaleY = 1.0f;
    this->digit.flags |= 0x1800;
    this->digit2.flags |= 0x1800;
    this->anm->InitSprite(&this->digit, 0x61);
    this->anm->InitSprite(&this->glyph, 0x20);
    this->digit.pos.z = 0.1f;
    this->highlight = 0;
    this->SetGlyphWidth(9);
}

int AsciiText::AddedCallback(AsciiText *text)
{
    memset(text, 0, sizeof(AsciiText));
    text->anm = g_SpriteEngine->PreloadAnm(1, "ascii.anm");
    if (text->anm == NULL)
    {
        return -1;
    }
    text->anm2 = g_SpriteEngine->PreloadAnm(3, "capture.anm");
    if (text->anm2 == NULL)
    {
        return -1;
    }
    text->Initialize();
    return 0;
}

void AsciiText::DrawStrings()
{
    float advance;
    int idx;
    AsciiString *string;
    BYTE *c;
    int clipped = 1;

    string = this->strings;
    this->glyph.flags |= 1;
    this->glyph.flags |= 0x1800;
    for (idx = 0; idx < this->stringCount; idx++, string++)
    {
        this->glyph.pos = string->pos;
        c = (BYTE *)string->text;
        this->glyph.scaleY = string->scaleX;
        this->glyph.scaleX = string->scaleY;
        advance = this->glyphWidth * string->scaleX;
        if (clipped != string->inArena)
        {
            clipped = string->inArena;
            g_SpriteEngine->FlushQuadBatch();
            if (clipped != 0)
            {
                g_App.SetView(0);
            }
            else
            {
                g_App.SetView(2);
            }
        }
        while (*c != 0)
        {
            if (*c == '\n')
            {
                this->glyph.pos.y += 14.0f * string->scaleY;
                this->glyph.pos.x = string->pos.x;
            }
            else if (*c != ' ')
            {
                this->glyph.spriteData = this->anm->GetSprite(*c - 0x20);
                this->glyph.color = string->color;
                g_SpriteEngine->DrawUnrotated(&this->glyph);
                this->glyph.pos.x += advance;
            }
            else
            {
                this->glyph.pos.x += advance;
            }
            c++;
        }
    }
    if (clipped != 0)
    {
        g_SpriteEngine->FlushQuadBatch();
        g_App.SetView(2);
    }
}

void AsciiText::AddPopup(int side, D3DXVECTOR3 *pos, int value, D3DCOLOR color)
{
    AsciiPopup *popup;
    int n;

    if (this->nextPopup >= 100)
    {
        this->nextPopup = 0;
    }
    popup = &this->popups[side][this->nextPopup];
    popup->active = 1;
    n = 0;
    if (value >= 0)
    {
        while (value != 0)
        {
            popup->digits[n] = value % 10;
            n++;
            value /= 10;
        }
    }
    else
    {
        popup->digits[n] = 10;
        n++;
    }
    if (n == 0)
    {
        popup->digits[n] = 0;
        n++;
    }
    popup->digitCount = n;
    popup->color = color;
    popup->timer = 0;
    popup->pos = *pos;
    this->nextPopup++;
}

void PauseMenu::DrawPauseMenu()
{
    int i;

    if (g_GameManager.isPaused)
    {
        g_SpriteEngine->FlushQuadBatch();
        g_App.SetView(2);
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].IsShown())
            {
                g_SpriteEngine->DrawUnrotated(&this->sprites[i]);
            }
        }
    }
}

void ContinueMenu::DrawContinueMenu()
{
    int i;

    if (g_GameManager.isInContinueMenu)
    {
        g_SpriteEngine->FlushQuadBatch();
        g_App.SetView(2);
        if ((g_App.stateFlags >> 1) & 1 && (this->state != 0 || this->stateFrames > 2))
        {
            g_SpriteEngine->DrawUnrotated(&this->background);
        }
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].IsShown())
            {
                g_SpriteEngine->DrawUnrotated(&this->sprites[i]);
            }
        }
    }
}

void AsciiText::DrawPopups(int side)
{
    AsciiPopup *popup;
    int i;
    int j;
    BYTE *digit;
    float dx;
    float dy;
    int distSq;
    int alpha;

    popup = this->popups[side];
    if (!g_App.GetNoFog())
    {
        g_App.DisableFog();
    }
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    for (i = 0; i < 100; i++, popup++)
    {
        if (!popup->active)
        {
            continue;
        }
        this->digit.pos.x = g_GameManager.ArenaToScreenX(popup->pos.x - popup->digitCount * 4);
        this->digit.pos.y = g_GameManager.ArenaToScreenY(popup->pos.y);
        this->digit.color = popup->color;
        dx = g_GameManager.sides[side].player->pos.x - popup->pos.x;
        dy = g_GameManager.sides[side].player->pos.y - popup->pos.y;
        distSq = dx * dx + dy * dy;
        if (distSq > 0x1000)
        {
            alpha = 0xd0;
        }
        else if (distSq > 0x400)
        {
            alpha = (distSq - 0x400) * 0x80 / 0xc00 + 0x50;
        }
        else
        {
            alpha = 0x50;
        }
        digit = &popup->digits[popup->digitCount - 1];
        for (j = popup->digitCount; j > 0; j--, digit--)
        {
            if (popup->timer < 0x34)
            {
                this->digit.spriteData = this->anm->GetSprite(*digit + 0x61);
                this->digit.alpha = alpha;
            }
            else if (popup->timer < 0x38)
            {
                this->digit.spriteData = this->anm->GetSprite(*digit + 0x61);
                this->digit.alpha = alpha;
            }
            else
            {
                this->digit.spriteData = this->anm->GetSprite(*digit + 0x61);
                this->digit.alpha = alpha;
            }
            this->digit.unknown28 = ((SpriteData *)this->digit.spriteData)->unknown34;
            g_SpriteEngine->DrawUnrotated(&this->digit);
            this->digit.pos.x += 8.0f;
        }
    }
}

void UnknownMenu::DrawUnknownMenu()
{
    int i;

    if (g_GameManager.unknown13e)
    {
        g_SpriteEngine->FlushQuadBatch();
        g_App.SetView(2);
        for (i = 0; i < ARRAY_SIZE(this->sprites); i++)
        {
            if (this->sprites[i].IsShown())
            {
                g_SpriteEngine->DrawUnrotated(&this->sprites[i]);
            }
        }
    }
}

int AsciiText::OnUpdate(AsciiText *text)
{
    int i;
    int side;
    AsciiPopup *entry;

    if (!g_GameManager.isPaused && !g_GameManager.isInContinueMenu)
    {
        if (!(g_GameManager.stateFlags & 0x1800))
        {
            for (side = 0; side < 2; side++)
            {
                entry = text->popups[side];
                if (g_GameManager.sides[side].flags & 1)
                {
                    continue;
                }
                for (i = 0; i < 100; i++, entry++)
                {
                    if (!entry->active)
                    {
                        continue;
                    }
                    entry->pos.y -= 0.2f * g_App.gameSpeed;
                    entry->timer++;
                    if (entry->timer > 60)
                    {
                        entry->active = 0;
                    }
                }
            }
        }
    }
    else if (g_GameManager.isPaused)
    {
        text->pauseMenu.UpdatePauseMenu();
    }
    if (g_GameManager.isInContinueMenu)
    {
        if (text->continueMenu.UpdateContinueMenu())
        {
            return 3;
        }
    }
    if (g_GameManager.unknown13e)
    {
        if (text->unknownMenu.UpdateUnknownMenu())
        {
            return 3;
        }
    }
    text->frameCounter++;
    return 1;
}

int AsciiText::OnDraw(AsciiText *text)
{
    text->DrawStrings();
    text->ClearStrings();
    text->pauseMenu.DrawPauseMenu();
    text->continueMenu.DrawContinueMenu();
    text->unknownMenu.DrawUnknownMenu();
    if (text->demoIcon.scriptIndex)
    {
        g_SpriteEngine->DrawUnrotated(&text->demoIcon);
    }
    return 1;
}

int AsciiText::OnDraw2(AsciiText *text)
{
    g_App.SetView(0);
    text->DrawPopups(0);
    g_App.SetView(1);
    text->DrawPopups(1);
    return 1;
}

int AsciiText::RegisterChain()
{
    AsciiText *text = &g_AsciiText;

    g_AsciiCalcChain.SetCallback((ChainCallback)AsciiText::OnUpdate);
    g_AsciiCalcChain.addedCallback = (ChainCallback)AsciiText::AddedCallback;
    g_AsciiCalcChain.deletedCallback = (ChainCallback)AsciiText::DeletedCallback;
    g_AsciiCalcChain.arg = text;
    if (g_Chain.AddToCalcChain(&g_AsciiCalcChain, 1) != 0)
    {
        return -1;
    }
    g_AsciiTextElemDraw.SetCallback((ChainCallback)AsciiText::OnDraw);
    g_AsciiTextElemDraw.arg = text;
    g_Chain.AddToDrawChain(&g_AsciiTextElemDraw, 0x22);
    g_AsciiDrawChain2.SetCallback((ChainCallback)AsciiText::OnDraw2);
    g_AsciiDrawChain2.arg = text;
    g_Chain.AddToDrawChain(&g_AsciiDrawChain2, 0x17);
    return 0;
}
