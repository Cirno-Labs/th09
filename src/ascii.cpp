// The ASCII text, from 0x434280, as th08's ascii.cpp.

#include <stdio.h>
#include <string.h>

#include "ascii.h"
#include "unsorted.h"

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
