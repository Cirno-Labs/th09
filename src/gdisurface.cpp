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

int GdiSurface::GetWidthBytes()
{
    return this->widthBytes;
}

int GdiSurface::GetWidth()
{
    return this->width;
}

int GdiSurface::GetHeight()
{
    return this->height;
}

bool GdiSurface::IsCreated()
{
    return this->bitmap != NULL;
}

GdiSurface::GdiSurface()
{
    this->format = -1;
    this->width = 0;
    this->height = 0;
    this->hdc = NULL;
    this->bitmap = NULL;
    this->oldObject = NULL;
    this->bits = NULL;
}

bool GdiSurface::Release()
{
    if (this->hdc != NULL)
    {
        SelectObject(this->hdc, this->oldObject);
        DeleteDC(this->hdc);
        DeleteObject(this->bitmap);
        this->format = -1;
        this->width = 0;
        this->height = 0;
        this->hdc = NULL;
        this->bitmap = NULL;
        this->oldObject = NULL;
        this->bits = NULL;
        return true;
    }
    return false;
}
