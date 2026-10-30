// A GDI DIB section text is drawn into, as th08's GdiSurface.

#pragma once

#include <windows.h>
#include <d3d8.h>

#include "common.h"

// A pixel format GdiSurface can create, as th08's.
struct FormatInfo
{
    // A D3DFORMAT; -1 ends the table.
    int format;
    int bitCount;
    DWORD alphaMask;
    DWORD redMask;
    DWORD greenMask;
    DWORD blueMask;
};
ASSERT_SIZE(FormatInfo, 0x18);

// The formats, as th08's (0x4a1ed8).
extern FormatInfo g_FormatInfos[7];

// A GDI DIB section text is drawn into before it is copied to a Direct3D
// surface; `format` is the D3DFORMAT its pixels follow, as th08's.
class GdiSurface
{
public:
    // th08's getters. Inline; the copies are at 0x435eb0 to 0x435f10.
    void *GetBits()
    {
        return this->bits;
    }
    int GetFormat()
    {
        return this->format;
    }
    HDC GetHdc()
    {
        return this->hdc;
    }
    int GetWidthBytes()
    {
        return this->widthBytes;
    }
    int GetWidth()
    {
        return this->width;
    }
    int GetHeight()
    {
        return this->height;
    }
    bool IsCreated()
    {
        return this->bitmap != NULL;
    }
    // (0x435f20, 0x436300)
    GdiSurface();
    ~GdiSurface();
    // Selects the old object back into `hdc`, then deletes the DC and the
    // bitmap. Returns whether there was a DC (0x435f40).
    bool Release();
    // The entry of g_FormatInfos for `format`, or the end of the table
    // when it is missing; NULL for -1, as th08's (0x435f90).
    FormatInfo *FindFormat(int format);
    // Creates the DIB section and its DC, after releasing any old ones
    // (0x436310).
    bool CreateInFormat(int width, int height, int format);
    // CreateInFormat, falling back to a 32-bit format for the 16-bit
    // ones, as th08's (0x436450).
    bool Create(int width, int height, int format);
    // Inverts the alpha of the rows from `y`, `width` by `height`; in
    // A1R5G5B5 it also shades the text, as th08's (0x435fe0).
    bool InvertAlpha(int x, int y, int width, int height, int lightShading);
    // Copies the pixels into `surface` when it has the same format.
    // Returns false when there is no bitmap or the surface can't be
    // locked (0x4361c0).
    bool CopyToSurface(IDirect3DSurface8 *surface);

    int format;
    int width;
    int height;
    DWORD imageSize;
    int widthBytes;
    HDC hdc;
    HGDIOBJ oldObject;
    HBITMAP bitmap;
    void *bits;
};
ASSERT_SIZE(GdiSurface, 0x24);

// The surface text is rendered through, as th08's (0x4dc544).
extern IDirect3DSurface8 *g_TextSurface;

// Creates and releases g_TextSurface, 1024 by 64 in A1R5G5B5 (0x4362c0,
// 0x4362e0).
void CreateTextSurface();
void ReleaseTextSurface();

// The colour of the text's outline; never written, so black (0x4dc548).
extern COLORREF g_TextOutlineColor;

// th08's RenderTextToTextureBold, but in the normal weight and copying two
// more pixels each way: draws `text` in `textColor`, outlined, on
// g_TextSurface at twice the size, then scales it into `texture`,
// `destHeight` rows high at row `y` (0x4364c0).
void RenderTextToTextureLarge(int x, int y, int width, int unused, int fontHeight, int destHeight,
                              COLORREF textColor, COLORREF shadowColor, char *text, IDirect3DTexture8 *texture);
// th08's RenderTextToTexture, for text 8 pixels high or less (0x4367b0).
void RenderTextToTexture(int x, int y, int width, int unused, int fontHeight, int destHeight, COLORREF textColor,
                         COLORREF shadowColor, char *text, IDirect3DTexture8 *texture);
