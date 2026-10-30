// The ASCII text, from 0x434280, as th08's ascii.cpp.

#include <stdio.h>
#include <string.h>

#include "ascii.h"

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
