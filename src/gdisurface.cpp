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

bool GdiSurface::CreateInFormat(int width, int height, int format)
{
    int widthBytes;
    HDC dc;
    HGDIOBJ oldObject;
    BITMAPV4HEADER bitmapInfo;
    FormatInfo *fmt;
    HBITMAP bitmap;
    void *data;

    this->Release();
    memset(&bitmapInfo, 0, sizeof(bitmapInfo));
    fmt = this->FindFormat(format);
    if (fmt == NULL)
    {
        return false;
    }
    widthBytes = (width * fmt->bitCount / 8 + 3) / 4 * 4;
    bitmapInfo.bV4Size = sizeof(bitmapInfo);
    bitmapInfo.bV4Width = width;
    bitmapInfo.bV4Height = -(height + 1);
    bitmapInfo.bV4Planes = 1;
    bitmapInfo.bV4BitCount = fmt->bitCount;
    bitmapInfo.bV4SizeImage = height * widthBytes;
    if (format != D3DFMT_X1R5G5B5 && format != D3DFMT_X8R8G8B8)
    {
        bitmapInfo.bV4V4Compression = BI_BITFIELDS;
        bitmapInfo.bV4RedMask = fmt->redMask;
        bitmapInfo.bV4GreenMask = fmt->greenMask;
        bitmapInfo.bV4BlueMask = fmt->blueMask;
        bitmapInfo.bV4AlphaMask = fmt->alphaMask;
    }
    bitmap = CreateDIBSection(NULL, (BITMAPINFO *)&bitmapInfo, DIB_RGB_COLORS, &data, NULL, 0);
    if (bitmap == NULL)
    {
        return false;
    }
    memset(data, 0, bitmapInfo.bV4SizeImage);
    dc = CreateCompatibleDC(NULL);
    oldObject = SelectObject(dc, bitmap);
    this->hdc = dc;
    this->bitmap = bitmap;
    this->bits = data;
    this->imageSize = bitmapInfo.bV4SizeImage;
    this->oldObject = oldObject;
    this->width = width;
    this->height = height;
    this->format = format;
    this->widthBytes = widthBytes;
    return true;
}

