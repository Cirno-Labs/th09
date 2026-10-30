// The ASCII text, from 0x434280, as th08's ascii.cpp.

#include <stdio.h>
#include <string.h>

#include "ascii.h"

void AsciiText::RemoveChain()
{
    g_Chain.Remove(&g_AsciiCalcChain);
    g_Chain.Remove(&g_AsciiTextElemDraw);
}
