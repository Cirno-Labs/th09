// Sprites run by ANM scripts, and the ANM files they come from. The
// layout follows th08's as far as th09's code shows it so far.

#pragma once

#include <d3dx8.h>

#include "common.h"
#include "timer.h"

class AnmLoaded;

// An argument of an ANM instruction, as th08's.
union AnmArg
{
    int i;
    float f;
    BYTE b[4];
};

// An instruction of an ANM script, as th08's.
struct AnmInstr
{
    short opcode;
    // The size of the instruction, arguments included.
    WORD size;
    // The frame of the script it runs on.
    short time;
    // Bit n: argument n names a script variable.
    WORD paramMask;
    AnmArg args[1];
};
struct SpriteData;

// The part of Sprite up to pos, as th08's. Its implicit constructor
// (0x4033a0) constructs the Timers; Sprite's calls it.
struct SpriteBase
{
    // The sprite's rotations about x, y and z, and their speeds.
    D3DXVECTOR3 rotation;
    D3DXVECTOR3 rotationSpeed;
    // Draw scales the sprite's width and height by these.
    float scaleX;
    float scaleY;
    // Added to the scales every frame.
    float scaleSpeedX;
    float scaleSpeedY;
    // The size of the sprite, from its SpriteData.
    float spriteWidth;
    float spriteHeight;
    // How far the texture coordinates scroll.
    float uvScrollX;
    float uvScrollY;
    // The script's frame.
    Timer scriptTime;
    Timer waitTimer;
    // The interpolations' frames so far and their lengths, as th08's.
    Timer interpDuration[7];
    Timer interpFrame[7];
    // The easings of the interpolations, as th08's.
    BYTE interpEasing;
    BYTE colorInterpEasing;
    BYTE alphaInterpEasing;
    BYTE rotationInterpEasing;
    BYTE scaleInterpEasing;
    BYTE color2InterpEasing;
    BYTE color2AlphaInterpEasing;
    unsigned char pad_ff[0x1];
    // The script's variables, as th08's: ResolveIntArgument and the
    // others read 10000 to 10009 in arguments as these.
    int scriptVar0;
    int scriptVar1;
    int scriptVar2;
    int scriptVar3;
    float scriptVarF0;
    float scriptVarF1;
    float scriptVarF2;
    float scriptVarF3;
    int scriptVar8;
    int scriptVar9;
    // Added to the texture scroll every frame.
    float uvScrollSpeedX;
    float uvScrollSpeedY;
    // The sprite's size scaled down by 256; drawMatrix is a copy of it.
    D3DXMATRIX matrix;
    D3DXMATRIX drawMatrix;
    // The sprite's size in the texture's size.
    D3DXMATRIX sizeMatrix;
    union
    {
        D3DCOLOR color;
        struct
        {
            BYTE colorRgb[3];
            BYTE alpha;
        };
    };
    // Drawn instead of color when bit 17 of flags is set.
    union
    {
        D3DCOLOR color2;
        struct
        {
            BYTE color2Rgb[3];
            BYTE color2Alpha;
        };
    };
    // th08's flags.
    union
    {
        DWORD flags;
        struct
        {
            DWORD shown : 1;
            DWORD visible : 1;
            // Set by SetRotationZ.
            // Set when the rotation or the scale changes, as th07's; the
            // draw functions then rebuild the matrix and clear both.
            DWORD rotationChanged : 1;
            DWORD scaleChanged : 1;
            DWORD blendMode : 2;
            DWORD unusedFlag6 : 1;
            DWORD unusedFlag7 : 1;
            DWORD usePosOffset : 1;
            DWORD flip : 2;
            DWORD anchor : 2;
            DWORD zWriteDisabled : 1;
            DWORD stopped : 1;
            DWORD cameraSetup : 1;
            DWORD freezeMatrix : 1;
            DWORD useColor2 : 1;
            DWORD unusedFlag18 : 1;
            DWORD scriptPaused : 1;
        };
    };
    // Nonzero: the sprite turns to face where it moves.
    short autoRotate;
    // The interrupt label to jump to on the next Step; 0 for none.
    short pendingInterrupt;
    // How SpriteEngine::DrawSprite draws the sprite, as th08's.
    int drawMode;
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
    AnmInstr *scriptStart;
    AnmInstr *scriptPos;
    SpriteData *spriteData;
    // Where an interrupt returns to.
    Timer interruptReturnTime;
    AnmInstr *interruptReturnPos;
    // The interpolations' ends, as th08's: the scale tween's x and y
    // are the start, z and colorTween.x the end; colorTween.y and .z
    // hold the color's start and end as D3DCOLORs.
    D3DXVECTOR3 moveStart;
    D3DXVECTOR3 moveEnd;
    D3DXVECTOR3 rotateStart;
    D3DXVECTOR3 rotateEnd;
    D3DVECTOR scaleTween;
    D3DVECTOR colorTween;
    D3DCOLOR color2TweenStart;
    D3DCOLOR color2TweenEnd;
    // Added to pos when it is drawn.
    D3DXVECTOR3 posOffset;
    int spriteTime;
    // The size of the characters text drawing uses; 15 from
    // AnmLoaded::StartScriptAtOrigin.
    BYTE fontWidth;
    BYTE fontHeight;
    unsigned char pad_29a[0xa];

    // `value`, or the variable it names when it is 10000 to 10009, as
    // th08's (0x436b80).
    int ResolveIntArgument(int value);
    // The variable `*value` names, when bit `bit` of `mask` marks it as
    // one, else `value`, as th08's: the float variables 10004 to 10007
    // (0x436c40), and the int ones (0x436cd0).
    float *ResolveFloatArgumentPtr(float *value, WORD mask, int bit);
    int *ResolveIntArgumentPtr(int *value, WORD mask, int bit);
    // ResolveIntArgument for floats; 10010 to 10012 are random numbers
    // (0x436e30).
    float ResolveFloatArgument(float value);

    // Start the move and the color interpolations: over `frames` with
    // `easing`, pos from `from` to `to`, and the color's RGB. Inline in
    // th08; these are the copies at 0x4233a3 and 0x4233f4, in an /O1
    // object.
    void SetMoveInterp(int frames, BYTE easing, D3DXVECTOR3 *from, D3DXVECTOR3 *to);
    void SetColorInterp(int frames, BYTE easing, D3DCOLOR from, D3DCOLOR to);

    // Sets pendingInterrupt. Inline; the copy is 0x406790.
    void SetPendingInterrupt(short interrupt)
    {
        this->pendingInterrupt = interrupt;
    }

    // Sets rotation.z to `angle` and rotationChanged (0x40f840).
    void SetRotationZ(float angle)
    {
        this->rotation.z = angle;
        this->rotationChanged = 1;
    }
    // Set blendMode to 1 and to 0 (0x41bd40, 0x41bd60).
    void SetBlendMode1()
    {
        this->blendMode = 1;
    }
    void ClearBlendMode()
    {
        this->blendMode = 0;
    }

    // Bit 0 of flags, as th08's. Inline; the copy is at 0x434370.
    BOOL IsShown()
    {
        return this->flags & 1;
    }
    // Clears bit 0 of flags, as th08's. Inline; the copy is at 0x434380.
    void ClearShown()
    {
        this->flags &= ~1;
    }

    // Resets the sprite's state up to pos, as th08's: a scale of 1,
    // white, the identity matrix, flags 7. Inline; the copy is at
    // 0x4127b0.
    void Initialize()
    {
        memset(this, 0, sizeof(SpriteBase));
        this->scaleX = 1.0f;
        this->scaleY = 1.0f;
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
    // Copied to Sprite's spriteWidth.
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

// An image stored in an ANM file, its pixels following the header, as
// th08's.
struct AnmImage
{
    unsigned char pad_0[0x6];
    // The pixels' format, an index into g_TextureFormats.
    short format;
    short width;
    short height;
    unsigned char pad_c[0x4];
    BYTE data[1];
};

// A sprite in an ANM file: its number and rectangle, in pixels, as
// th08's.
struct AnmSprite
{
    int id;
    float x;
    float y;
    float width;
    float height;
};

// An ANM file's header, as th08's. The sprites' and then the scripts'
// offsets follow it.
struct AnmFile
{
    int numSprites;
    int numScripts;
    int textureIndex;
    int width;
    int height;
    int format;
    D3DCOLOR colorKey;
    // The image's file name, or "@" for a texture created blank.
    int nameOffset;
    int spriteOffset;
    int alphaNameOffset;
    // 3 in th09's files.
    int version;
    // The texture's resource priority.
    DWORD priority;
    // The image stored in the file, when hasImage is set.
    int imageOffset;
    BYTE hasImage;
    BYTE ownsData;
    unsigned char pad_36[0x2];
    // The offset of the next ANM in the same file, 0 for the last.
    int nextOffset;
    unsigned char pad_3c[0x4];
    int spriteOffsets[1];
};

class AnmLoaded
{
public:
    // Starts script `script` of this file on `vm`, at the origin with no
    // offset (0x401560).
    void StartScriptAtOrigin(Sprite *vm, int script)
    {
        vm->scriptIndex = script;
        vm->pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        vm->posOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        vm->fontHeight = 15;
        vm->fontWidth = 15;
        this->SetScript(vm, this->scripts[script]);
    }
    // Starts script `script` of this file on `vm`, as th08's (0x403e00).
    void StartScript(Sprite *vm, int script)
    {
        vm->anm = this;
        vm->scriptIndex = script;
        this->SetScript(vm, this->scripts[script]);
    }
    // Starts `script` on `vm`, or zeroes `vm` if there is no script or
    // the file is still loading (0x4395a0).
    void SetScript(Sprite *vm, void *script);
    // Starts script `script` + n of this file on sprite n of the
    // `count` sprites from `vms`, and makes their sprites their base
    // sprites, as th08's (0x439630).
    void StartSpriteSeries(Sprite *vms, int script, int count);
    // Shows sprite `sprite` of this file on `vm`, as th08's. Returns 0,
    // or -1 while the file is not loaded (0x436ac0).
    int SetSprite(Sprite *vm, int sprite);
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
    AnmFile *file;
    // How many ANMs the file chains.
    int anmCount;
    SpriteData *sprites;
    void **scripts;
    AnmTexture *textures;
    // While the textures are created a frame at a time: one more than
    // the number of ANMs done, 0 when all are.
    int loadStep;
    // Set by PreloadAnmFile to have StepAnmLoading release the file.
    int releaseRequested;
    // The path it was loaded from.
    char path[0x100];
};
ASSERT_SIZE(AnmLoaded, 0x120);

// A vertex with a transformed position, a color and texture
// coordinates, as th08's.
struct VertexTex1DiffuseXyzrwh
{
    D3DXVECTOR3 pos;
    float w;
    D3DCOLOR diffuse;
    struct
    {
        float x;
        float y;
    } uv;
};

// A vertex with a transformed position and a color, as th08's.
struct VertexDiffuseXyzrwh
{
    D3DXVECTOR3 pos;
    float w;
    D3DCOLOR diffuse;
};

// A vertex with a transformed position and texture coordinates, as
// th08's.
struct VertexTex1Xyzrwh
{
    D3DXVECTOR3 pos;
    float w;
    struct
    {
        float x;
        float y;
    } uv;
};

// A vertex with an untransformed position and texture coordinates, as
// th08's.
struct VertexTex1Xyz
{
    D3DXVECTOR3 pos;
    D3DXVECTOR2 uv;
};

// A vertex with an untransformed position, a color and texture
// coordinates, as th08's.
struct VertexTex1DiffuseXyz
{
    D3DXVECTOR3 pos;
    D3DCOLOR diffuse;
    D3DXVECTOR2 uv;
};

// Runs every sprite's script (th08's name), at *0x4dc550.
class SpriteEngine
{
public:
    // Zeroes the engine and sets up g_TexQuad and g_DiffuseQuad, as
    // th08's (0x43ce30).
    SpriteEngine();
    // Empty; the linker folded it with DebugPrint (0x43d2b0).
    ~SpriteEngine();
    // Forgets the texture last set, so that the next sprite sets it
    // again (0x4015f0).
    void ClearTexture()
    {
        this->currentTexture = NULL;
    }
    // Sets lastCameraSetupFlag (0x401600).
    void SetLastCameraSetupFlag(BYTE value)
    {
        this->lastCameraSetupFlag = value;
    }
    // Stops modulating sprites by a color (0x401610).
    void ResetColorModulation()
    {
        this->modulateColorEnabled = 0;
        this->modulateColor = 0x80808080;
    }
    // Modulates sprites by `color` (0x401620).
    void SetColorModulation(D3DCOLOR color)
    {
        this->modulateColorEnabled = 1;
        this->modulateColor = color;
    }
    // Forgets the blend mode last set, as ClearTexture (0x40e370).
    void ClearBlendMode()
    {
        this->blendMode = 3;
    }
    // The same for the vertex format (0x40e380).
    void ClearVertexFormat()
    {
        this->vertexFormat = 0xff;
    }
    // The same for the sprite data (0x40e390).
    void ClearSpriteData()
    {
        this->currentSpriteData = NULL;
    }
    // The same for the color operation (0x415d00).
    void ClearColorOp()
    {
        this->colorOp = 0xff;
    }
    // The same for the z write state (0x415d10).
    void ClearZWrite()
    {
        this->zWriteDisabled = 0xff;
    }
    // The same for lastCameraSetupFlag (0x42d2e0).
    void ClearLastCameraSetupFlag()
    {
        this->lastCameraSetupFlag = 0xff;
    }
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
    // Reads the ANM file at `path` into slot `slot`, sets up its tables
    // and reads its image files, as th08's; a file in the slot is
    // released by StepAnmLoading first. The textures are created later,
    // by PostloadAnm. Returns the slot's AnmLoaded, or NULL on failure
    // (0x43c610).
    AnmLoaded *PreloadAnmFile(int slot, char *path);
    // Loads the ANM file at `path` into slot `slot` at once, as th08's.
    // Returns it, or NULL on failure (0x43cf90).
    AnmLoaded *LoadAnm(int slot, char *path);
    // Loads `path` into ANM slot `slot`, as th08's, waiting for the
    // loading thread. Returns it, or NULL on failure (0x43c7a0).
    AnmLoaded *PreloadAnm(int slot, char *path);
    // Creates the textures and sprites of ANM `index` of `anm`, its
    // header `file`, numbering them from `spriteOffset` and
    // `scriptOffset`, as th08's. Returns 1, or -1 on failure (0x43c800).
    int LoadAnmSprites(AnmLoaded *anm, int index, int spriteOffset, int scriptOffset, AnmFile *file);
    // Frees ANM slot `slot`, as th08's (0x43ca30).
    void ReleaseAnm(int slot);
    // Loads the image at `path` as surface `surface`, as th08's
    // (0x43cae0).
    int LoadSurface(int surface, char *path);
    // Reads the image file at `path` for LoadSurface to load surface
    // `surface` from later, as th08's. Returns 0, or -1 on failure
    // (0x43ccd0).
    int PreloadSurface(int surface, char *path);
    // Creates the textures and sprites of the next ANM of `anm`, as
    // th08's: one a call, counted by anm->loadStep, which drops to 0
    // after the last. Returns `anm`, or NULL on failure (0x43cd40).
    AnmLoaded *PostloadAnm(AnmLoaded *anm);
    // Releases the ANM files asked to be, and goes on creating the
    // textures of those loading. Returns 0, or -1 on failure (0x43cdd0).
    int StepAnmLoading();
    // Draws `vm` the way its drawMode asks for, as th08's (0x436d70).
    void DrawSprite(Sprite *vm);
    // Draws `vm` at its position without rotating it, as th08's
    // (0x43a950).
    int DrawUnrotated(Sprite *vm);
    // The same, not rounding its position (0x43afe0).
    int DrawNoRound(Sprite *vm);
    // DrawUnrotated with the texture mirrored left to right (0x43aa50).
    int DrawUnrotatedMirrored(Sprite *vm);
    // Draws `vm` rotated, not rounding its position (0x43ad40).
    int DrawRotatedNoRound(Sprite *vm);
    // Works out the corners of `vm`'s quad on screen from its 3D
    // position, as th08's. Returns 0, or -1 if it is out of the depth
    // range (0x439820).
    int ProjectSprite(Sprite *vm);
    // The same for `vm` as a quad in 3D, as th08's (0x439a90).
    void ProjectSprite3D(Sprite *vm);
    // Sets the render state for `vm` and draws the projected quad, as
    // th08's (0x43a0f0).
    int DrawQuad(Sprite *vm, int flags);
    // Draws the `count` vertices of `vertices` as a triangle strip with
    // `vm`'s texture, as th08's. Returns 0, or -1 if `vm` is hidden
    // (0x43b4d0).
    int DrawVertexStrip(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count);
    // Draws `vm` in 3D with the vertex buffer's quad, as th08's
    // (0x43b1a0).
    int Draw3D(Sprite *vm);
    // Batches `quad`, set up by the caller, with `vm`'s texture, as
    // th08's (0x43b5c0).
    int DrawVertexQuad(Sprite *vm, VertexTex1DiffuseXyzrwh *quad);
    // Draws the `count` vertices of `vertices` untextured, with `vm`'s
    // blend mode, as a triangle strip (0x43b670), and as a fan, as
    // th08's DrawColoredFan (0x43b7b0). Return 0.
    int DrawColoredStrip(Sprite *vm, VertexDiffuseXyzrwh *vertices, int count);
    int DrawColoredFan(Sprite *vm, VertexDiffuseXyzrwh *vertices, int count);
    // Draws the `count` vertices of `vertices` as a triangle fan with
    // `vm`'s texture and no z writes. Returns 0 (0x43b8f0).
    int DrawVertexFan(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count);
    // Sets up quad, g_DiffuseQuadXyz and the vertex buffer holding quad,
    // as th08's (0x43b9c0).
    void CreateQuad();
    // Sets the blend mode, the texture factor color, the z writes and the
    // camera for `vm`, as th08's (0x439e90).
    void SetRenderStateForSprite(Sprite *vm);
    // Sets the texture coordinates, color and w of the `count` vertices
    // of a triangle strip across `vm`'s sprite, as th08's. Returns 0, or
    // -1 for fewer than 3 (0x439cf0).
    int FillSpriteStripVertices(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count);
    // The same for a strip down the sprite, as th08's (0x439dc0).
    int FillVerticalStripVertices(Sprite *vm, VertexTex1DiffuseXyzrwh *vertices, int count);
    // Sets the blend mode and the z writes for `vm`, as th08's
    // (0x43a030).
    void SetBlendModeForSprite(Sprite *vm);
    // DrawQuad with the texture mirrored left to right (0x43a520).
    int DrawQuadMirrored(Sprite *vm, int flags);
    // Draws `vm` projected from its 3D position, as th08's (0x43b0e0).
    int DrawProjected(Sprite *vm);
    // Draws `vm` as a quad in 3D, as th08's (0x43b140).
    int DrawProjected3D(Sprite *vm);
    // Runs `vm`'s script up to its next wait (0x436f30).
    int Step(Sprite *vm);
    // Rotates (`x`, `y`) by the angle with `sine` and `cosine` and moves
    // it to (`cx`, `cy`), into `out`, as th08's (0x4397e0).
    void RotatePoint(float *out, float x, float y, float sine, float cosine, float cx, float cy);
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
    // Releases the surfaces before the device is reset, as th08's
    // (0x42d2f0).
    void ReleaseSurfaces();
    // Runs the pending copies of the back buffer to a texture and to a
    // surface, as th08's (0x42d320).
    void DoPendingCopy();
    // Copies a rectangle of the back buffer to one of the first texture
    // of the ANM file in slot `texture`, as th08's (0x43c200).
    void CopyBackBufferToTexture(int texture, int srcLeft, int srcTop, int srcWidth, int srcHeight, int dstLeft,
                                 int dstTop, int dstWidth, int dstHeight);
    // Creates surface `surface` and its backup from a rectangle of the
    // back buffer, as th08's (0x43c2d0).
    void CopyBackBufferToSurface(int surface, int srcLeft, int srcTop, int srcWidth, int srcHeight, int dstLeft,
                                 int dstTop, int dstWidth, int dstHeight);
    // Creates `tex`'s texture at `width` by `height` in the format
    // g_TextureFormats[`format`], as th08's. Returns 0 (0x43bba0).
    int CreateTexture(AnmTexture *tex, int width, int height, int format);
    // Checks `file`, ANM `index` of `anm`, and reads its image file
    // unless the image is in the ANM or made blank, as th08's. Returns 1,
    // or -1 on failure (0x43bbe0).
    int PreloadAnmTexture(AnmLoaded *anm, int index, int *spriteIndex, int *scriptIndex, AnmFile *file);
    // Creates `tex`'s texture from its image file, in the format
    // g_TextureFormats[`format`], with `colorKey` transparent, as th08's.
    // Returns 0, or -1 on failure (0x43c470).
    int LoadTexture(AnmTexture *tex, int format, D3DCOLOR colorKey);
    // Creates `tex`'s texture from the pixels of `image`, in the format
    // g_TextureFormats[`format`], as th08's. Returns 0, or -1 on failure
    // (0x43c4c0).
    int CreateTextureFromImage(AnmTexture *tex, AnmImage *image, int format);
    // Sends interrupt `interrupt` to the `count` sprites from `vms` that
    // show a loaded sprite, as th08's (0x436de0).
    void SendInterruptToLoadedSprites(Sprite *vms, int count, short interrupt);
    // Runs `count` sprites from `vms` a frame, as th08's (0x439560).
    void StepSprites(Sprite *vms, int count);
    // Empties the quad batch, as th08's (0x439680).
    void BeginQuadBatch();
    // Draws the sprites batched so far (0x4396a0).
    void FlushQuadBatch();
    // Adds `quad` to the batch as two triangles, as th08's. Returns 0
    // (0x439730).
    int AddQuadToBatch(VertexTex1DiffuseXyzrwh *quad);
    // Draws `vm`, as th08's (0x43ab50).
    int Draw(Sprite *vm);
    // Draws `vm`, then the batched quads, as th08's. Inline; the copy is
    // at 0x401640.
    void DrawAndFlush(Sprite *vm)
    {
        this->Draw(vm);
        this->FlushQuadBatch();
    }
    // Inline, even in /Ob0 files that emit the copy at 0x401660:
    // callers know this does not change g_SpriteEngine.
    AnmLoaded *GetAnm(int slot)
    {
        return &this->anms[slot];
    }
    // The file of the ANM in slot `slot` (0x41bd70).
    AnmFile *GetAnmFile(int slot)
    {
        return this->GetAnm(slot)->file;
    }
    // Whether `vm` shows a sprite whose ANM file has its textures
    // loaded, as th08's. Inline; the copy at 0x423469 is the one
    // MainMenu::OnDraw's /O1 object emits.
    BOOL HasTexture(Sprite *vm)
    {
        if (vm->spriteData == NULL)
        {
            return FALSE;
        }
        if (vm->spriteData->sourceFileIndex < 0)
        {
            return FALSE;
        }
        return this->anms[vm->spriteData->sourceFileIndex].textures != NULL;
    }
    // The image loaded as surface `surface`, as th08's. Inline; the copy
    // is App's object's (0x42f400).
    IDirect3DSurface8 *GetSurface(int surface)
    {
        return this->surfaces[surface];
    }
    // Releases vertexBuffer, as th08's (0x42f3d0).
    void ReleaseVertexBuffer();

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
    // The world matrix ProjectSprite3D last used.
    D3DXMATRIX lastWorldMatrix;
    Sprite unknownSprite;
    unsigned char pad_12308[0x123fc - 0x12308];
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
    SpriteData *currentSpriteData;
    // Holds quad, for drawing sprites in 3D.
    IDirect3DVertexBuffer8 *vertexBuffer;
    // The quad of a sprite 256 pixels across, as th08's.
    VertexTex1Xyz quad[4];
    // The number of quads batched to draw with FlushQuadBatch.
    int batchQuadCount;
    // The batched quads, two triangles each.
    VertexTex1DiffuseXyzrwh batchVertices[0x18000];
    // Where the next quad goes, and the first one not drawn yet.
    VertexTex1DiffuseXyzrwh *batchCursor;
    VertexTex1DiffuseXyzrwh *batchStart;
    // -1 at first, as th08's pendingTexCopyTexture.
    int pendingTexCopyTexture;
    // The rest of the parameters of the copies, as th08's.
    int pendingTexCopySrcLeft;
    int pendingTexCopySrcTop;
    int pendingTexCopySrcWidth;
    int pendingTexCopySrcHeight;
    int pendingTexCopyDstLeft;
    int pendingTexCopyDstTop;
    int pendingTexCopyDstWidth;
    int pendingTexCopyDstHeight;
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

// A quad with texture coordinates, as th08's (0x4dc5c8).
extern VertexTex1Xyzrwh g_TexQuad[4];

// quad, with a color, as th08's (0x4dc628).
extern VertexTex1DiffuseXyz g_DiffuseQuadXyz[4];

// 0.5f, which DrawQuad's rounding subtracts (0x490f54).
extern const float g_Half;

// The screen corners of the quad the draw functions build, as th08's
// (0x4dc558).
extern VertexTex1DiffuseXyzrwh g_DiffuseQuad[4];

// The Direct3D formats of ANM images, by index (0x490f58).
extern const D3DFORMAT g_TextureFormats[];
// The bytes per pixel of each of g_TextureFormats (0x490f70).
extern const int g_TextureFormatSizes[];
