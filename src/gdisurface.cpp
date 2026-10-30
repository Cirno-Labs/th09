// GdiSurface, from 0x435eb0, as th08's unsorted43efef.cpp.

#include <string.h>

#include "gdisurface.h"
#include "unsorted.h"

HDC GdiSurface::GetHdc()
{
    return this->hdc;
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

bool GdiSurface::CopyToSurface(IDirect3DSurface8 *surface)
{
    BYTE *dst;
    int lineSize;
    RECT region;
    int row;
    int srcPitch;
    D3DSURFACE_DESC surfaceDesc;
    BYTE *src;
    D3DLOCKED_RECT lockedSurface;

    if (!this->IsCreated())
    {
        return false;
    }
    surface->GetDesc(&surfaceDesc);
    region.left = 0;
    region.top = 0;
    region.right = this->GetWidth();
    region.bottom = this->GetHeight();
    if (surface->LockRect(&lockedSurface, &region, 0) != D3D_OK)
    {
        return false;
    }
    lineSize = lockedSurface.Pitch;
    srcPitch = this->GetWidthBytes();
    src = (BYTE *)this->GetBits();
    dst = (BYTE *)lockedSurface.pBits;
    if (surfaceDesc.Format == this->GetFormat())
    {
        for (row = 0; row < this->GetHeight(); row++)
        {
            memcpy(dst, src, srcPitch);
            src += srcPitch;
            dst += lineSize;
        }
    }
    surface->UnlockRect();
    return true;
}

void CreateTextSurface()
{
    g_App.d3dDevice->CreateImageSurface(1024, 64, D3DFMT_A1R5G5B5, &g_TextSurface);
}

void ReleaseTextSurface()
{
    if (g_TextSurface != NULL)
    {
        g_TextSurface->Release();
        g_TextSurface = NULL;
    }
}

GdiSurface::~GdiSurface()
{
    this->Release();
}

