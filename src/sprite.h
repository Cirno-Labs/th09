// Sprites run by ANM scripts, and the ANM files they come from. The
// layout follows th08's as far as th09's code shows it so far.

#pragma once

#include <d3dx8.h>

#include "common.h"
#include "timer.h"

class AnmLoaded;
struct SpriteData;

// The part of Sprite up to pos, as th08's. Its implicit constructor
// (0x4033a0) constructs the Timers; Sprite's calls it.
struct SpriteBase
{
    // The sprite's rotations about x, y and z, and their speeds.
    D3DXVECTOR3 rotation;
    D3DXVECTOR3 rotationSpeed;
    // Draw scales the sprite's height and width by these.
    float scaleY;
    float scaleX;
    unsigned char pad_20[0x28 - 0x20];
    // Copied from the sprite's SpriteData.
    float unknown28;
    unsigned char pad_2c[0x38 - 0x2c];
    // The script's frame.
    Timer scriptTime;
    Timer waitTimer;
    // The interpolations' frames so far and their lengths, as th08's.
    Timer interpDuration[7];
    Timer interpFrame[7];
    unsigned char pad_f8[0x130 - 0xf8];
    D3DXMATRIX matrix;
    unsigned char pad_170[0x1f0 - 0x170];
    union
    {
        D3DCOLOR color;
        struct
        {
            BYTE colorRgb[3];
            BYTE alpha;
        };
    };
    unsigned char pad_1f4[0x1f8 - 0x1f4];
    DWORD flags;
    unsigned char pad_1fc[0x1fe - 0x1fc];
    // The interrupt label to jump to on the next Step; 0 for none.
    short pendingInterrupt;
    unsigned char pad_200[0x204 - 0x200];
    // The ANM file whose script runs.
    AnmLoaded *anm;
};
ASSERT_SIZE(SpriteBase, 0x208);

// One sprite and the state of the ANM script that drives it, as th08's.
class Sprite : public SpriteBase
{
public:
    // Inline, as th08's; the copy is at 0x403a10.
    Sprite()
    {
        memset(this, 0, sizeof(Sprite));
        this->spriteIndex = -1;
    }

    D3DXVECTOR3 pos;
    short spriteIndex;
    // The slot of the ANM file whose script runs.
    short anmSlot;
    short baseSpriteIndex;
    // The script AnmLoaded::StartScriptAtOrigin started.
    short scriptIndex;
    // The script SetScript started, and the instruction Step runs next.
    void *scriptStart;
    void *scriptPos;
    SpriteData *spriteData;
    Timer interruptReturnTime;
    unsigned char pad_234[0x288 - 0x234];
    // Added to pos when it is drawn.
    D3DXVECTOR3 posOffset;
    int spriteTime;
    // The size of the characters text drawing uses; 15 from
    // AnmLoaded::StartScriptAtOrigin.
    BYTE fontWidth;
    BYTE fontHeight;
    unsigned char pad_29a[0xa];

    // Sets pendingInterrupt. Inline; the copy is 0x406790.
    void SetPendingInterrupt(short interrupt)
    {
        this->pendingInterrupt = interrupt;
    }

    // Bit 0 of flags, as th08's. Inline; the copy is at 0x434370.
    BOOL IsShown()
    {
        return this->flags & 1;
    }

    // Resets the sprite's state up to pos, as th08's: a scale of 1,
    // white, the identity matrix, flags 7. Inline; the copy is at
    // 0x4127b0.
    void Initialize()
    {
        memset(this, 0, sizeof(SpriteBase));
        this->scaleY = 1.0f;
        this->scaleX = 1.0f;
        this->color = 0xffffffff;
        D3DXMatrixIdentity(&this->matrix);
        *(WORD *)&this->flags = 7;
        this->scriptTime.Initialize();
    }
};
ASSERT_SIZE(Sprite, 0x2a4);

// An ANM file, its sprites and its scripts, as th08's.
// A sprite of an ANM file, as th08's.
struct SpriteData
{
    // The slot of the ANM file the sprite comes from.
    int sourceFileIndex;
    // The texture it is drawn from.
    IDirect3DTexture8 *texture;
    // The sprite's rectangle in the texture, in pixels.
    float startX;
    float startY;
    float endX;
    float endY;
    // The texture's size.
    float textureHeight;
    float textureWidth;
    // The rectangle in texture coordinates.
    float uvStartX;
    float uvStartY;
    float uvEndX;
    float uvEndY;
    // Its size in pixels.
    float height;
    // Copied to Sprite's unknown28.
    float width;
    // How much larger the texture is than the sprite's size.
    float scaleX;
    float scaleY;
    int unusedInt40;
};
ASSERT_SIZE(SpriteData, 0x44);

// A texture of an ANM file, and the image it is created from, as th08's.
struct AnmTexture
{
    IDirect3DTexture8 *texture;
    void *data;
    int size;
};

class AnmLoaded
{
public:
    // Starts script `script` of this file on `vm`, at the origin with no
    // offset (0x401560).
    void StartScriptAtOrigin(Sprite *vm, int script);
    // Starts `script` on `vm`, or zeroes `vm` if there is no script or
    // the file is still loading (0x4395a0).
    void SetScript(Sprite *vm, void *script);
    // Shows sprite `sprite` of this file on `vm` (0x436ac0).
    void SetSprite(Sprite *vm, int sprite);
    // Zeroes `vm` and shows sprite `sprite` of this file on it, as
    // th08's. Inline; the copy is at 0x412800.
    void InitSprite(Sprite *vm, int sprite)
    {
        vm->Initialize();
        vm->anm = this;
        this->SetSprite(vm, sprite);
    }
    // Copies `sprite` into sprite `index` and works out its texture
    // coordinates and size, as th08's (0x43bcb0).
    void LoadSprite(int index, SpriteData *sprite);

    // Sprite `sprite` of this file, as th08's. Inline; the copy is at
    // 0x434390.
    SpriteData *GetSprite(int sprite)
    {
        return &this->sprites[sprite];
    }

    // Its slot in SpriteEngine's files.
    int slot;
    void *file;
    int anmCount;
    SpriteData *sprites;
    void **scripts;
    AnmTexture *textures;
    // Nonzero while the textures are still being created.
    int loadStep;
    unsigned char pad_1c[0x120 - 0x1c];
};
ASSERT_SIZE(AnmLoaded, 0x120);

// Runs every sprite's script (th08's name), at *0x4dc550.
class SpriteEngine
{
public:
    // Forgets the texture last set, so that the next sprite sets it
    // again (0x4015f0).
    void ClearTexture();
    // Sets lastCameraSetupFlag (0x401600).
    void SetLastCameraSetupFlag(BYTE value);
    // Stops modulating sprites by a color (0x401610).
    void ResetColorModulation();
    // Modulates sprites by `color` (0x401620).
    void SetColorModulation(D3DCOLOR color);
    // Forgets the blend mode last set, as ClearTexture (0x40e370).
    void ClearBlendMode();
    // The same for the vertex format (0x40e380).
    void ClearVertexFormat();
    // The same for the sprite data (0x40e390).
    void ClearSpriteData();
    // The same for the color operation (0x415d00).
    void ClearColorOp();
    // The same for the z write state (0x415d10).
    void ClearZWrite();
    // The same for lastCameraSetupFlag (0x42d2e0).
    void ClearLastCameraSetupFlag();
    // Has a copy of the back buffer to surface `texture` done after the
    // frame is drawn, unless one is already pending: then returns -1,
    // else 0. Inline in th08; this copy is 0x42f420.
    int SetPendingSurfaceCopy(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight, int dstLeft,
                              int dstTop, int dstWidth, int dstHeight);
    // Clears the counters of the frame. Inline, as th08's; the copy is
    // 0x42f760.
    void ClearDrawStats()
    {
        this->stepCount = 0;
        this->renderStateSetCount = 0;
        this->scriptStartCount = 0;
        this->batchDrawCount = 0;
    }
    // Forgets the render state last set, clears the counters and the
    // color modulation and stops the shake, as th08's App::OnUpdate does
    // every frame (0x42f770).
    void ResetFrameState();
    // Loads `path` into ANM slot `slot`, as th08's, waiting for the
    // loading thread. Returns it, or NULL on failure (0x43c7a0).
    AnmLoaded *PreloadAnm(int slot, char *path);
    // Frees ANM slot `slot`, as th08's (0x43ca30).
    void ReleaseAnm(int slot);
    // Draws `vm` at its position without rotating it, as th08's
    // (0x43a950).
    int DrawUnrotated(Sprite *vm);
    // Runs `vm`'s script up to its next wait (0x436f30).
    void Step(Sprite *vm);
    // Releases `tex`'s texture and frees its image, as th08's
    // (0x43bc80).
    void ReleaseTexture(AnmTexture *tex);
    // Renders `text` into `texture`, its font sizes 15 when not
    // positive and scaled by `scaleX` and `scaleY`, as th08's; fonts of
    // 8 pixels or less are drawn 8 by 8 with RenderTextToTexture
    // (0x43bd40).
    void RenderTextToTextureIndex(IDirect3DTexture8 *texture, int x, int y, int width, int unused, int fontHeight,
                                  int destHeight, D3DCOLOR textColor, D3DCOLOR shadowColor, char *text, float scaleX,
                                  float scaleY);
    // Renders printf-style text into `sprite`'s texture, as th08's
    // (0x43bdc0).
    void RenderText(Sprite *sprite, D3DCOLOR textColor, D3DCOLOR shadowColor, char *fmt, ...);
    // RenderText, centred in the sprite, as th08's (0x43be50).
    void RenderTextCentered(Sprite *vm, D3DCOLOR color, D3DCOLOR shadowColor, char *fmt, ...);
    // Releases surface `surface`, its backup and its file, as th08's
    // (0x43bf30).
    void ReleaseSurface(int surface);
    // Copies surface `surface`, from (`left`, `top`) to its end, to
    // (`x`, `y`) on the back buffer, first creating it from its backup
    // if needed, as th08's (0x43bfa0).
    void CopySurfaceToBackBuffer(int surface, int left, int top, int x, int y);
    // The same for the `width` by `height` rectangle at (`left`, `top`),
    // as th08's (0x43c0d0).
    void CopySurfaceRectToBackBuffer(int surface, int x, int y, int left, int top, int width, int height);
    // Copies a rectangle of the back buffer to one of the first texture
    // of the ANM file in slot `texture`, as th08's (0x43c200).
    void CopyBackBufferToTexture(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight, int dstLeft,
                                 int dstTop, int dstWidth, int dstHeight);
    // Draws the sprites batched so far (0x4396a0).
    void FlushQuadBatch();
    // Draws `vm`, as th08's (0x43ab50).
    int Draw(Sprite *vm);
    // Draws `vm`, then the batched quads, as th08's. Inline; the copy is
    // at 0x401640.
    void DrawAndFlush(Sprite *vm)
    {
        this->Draw(vm);
        this->FlushQuadBatch();
    }
    // The ANM file in slot `slot`, as th08's (0x401660).
    AnmLoaded *GetAnm(int slot);
    // The image loaded as surface `surface`, as th08's. Inline in th08;
    // this copy is 0x42f400.
    IDirect3DSurface8 *GetSurface(int surface);

    D3DCOLOR modulateColor;
    int modulateColorEnabled;
    // The surface SetPendingSurfaceCopy's copy goes to; negative for
    // none. The rest of the copy's parameters are at the end.
    int pendingCopyTexture;
    // How many scripts SetScript started.
    int scriptStartCount;
    // Counters, cleared every frame, as th08's.
    int stepCount;
    int renderStateSetCount;
    int batchDrawCount;
    // How far the screen shakes.
    float shakeX;
    float shakeY;
    // The loaded ANM files, by slot.
    AnmLoaded anms[0x100];
    unsigned char pad_12024[0x123fc - 0x12024];
    // Images loaded as surfaces to copy to the back buffer from, as
    // th08's.
    IDirect3DSurface8 *surfaces[0x20];
    // System memory copies of them, to recreate them from.
    IDirect3DSurface8 *surfacesBackup[0x20];
    // The image files they are loaded from, and their sizes.
    void *surfaceFiles[0x20];
    int surfaceFileSizes[0x20];
    // The images' sizes and formats.
    D3DXIMAGE_INFO surfaceInfos[0x20];
    // The texture factor last set on the device.
    D3DCOLOR currentColor;
    // The render state last set, so that sprites only change it when
    // they need to.
    IDirect3DTexture8 *currentTexture;
    BYTE blendMode;
    BYTE colorOp;
    BYTE vertexFormat;
    BYTE zWriteDisabled;
    BYTE lastCameraSetupFlag;
    unsigned char pad_12889[0x1288c - 0x12889];
    // The sprite data last drawn.
    void *currentSpriteData;
    unsigned char pad_12890[0x2b2914 - 0x12890];
    int pendingCopySrcLeft;
    int pendingCopySrcTop;
    int pendingCopySrcWidth;
    int pendingCopySrcHeight;
    int pendingCopyDstLeft;
    int pendingCopyDstTop;
    int pendingCopyDstWidth;
    int pendingCopyDstHeight;
};
extern SpriteEngine *g_SpriteEngine;
