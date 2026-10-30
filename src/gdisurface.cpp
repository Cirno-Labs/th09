// GdiSurface, from 0x435eb0, as th08's unsorted43efef.cpp.

#include <string.h>

#include "gdisurface.h"
#include "unsorted.h"

void *GdiSurface::GetBits()
{
    return this->bits;
}

int GdiSurface::GetFormat()
{
    return this->format;
}

HDC GdiSurface::GetHdc()
{
    return this->hdc;
}
