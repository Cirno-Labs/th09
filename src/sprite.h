// Sprites run by ANM scripts, and the ANM files they come from. The
// layout follows th08's as far as th09's code shows it so far.

#pragma once

#include <d3dx8.h>

#include "common.h"
#include "timer.h"

class AnmLoaded;

// One sprite and the state of the ANM script that drives it, as th08's.
class Sprite
{
public:
    // The sprite's rotations about x, y and z, and their speeds.
    D3DXVECTOR3 rotation;
    D3DXVECTOR3 rotationSpeed;
    // Draw scales the sprite's height and width by these.
    float scaleY;
    float scaleX;
    unsigned char pad_20[0x38 - 0x20];
    // The script's frame.
    Timer scriptTime;
    unsigned char pad_44[0x130 - 0x44];
    D3DXMATRIX matrix;
    unsigned char pad_170[0x1f0 - 0x170];
    D3DCOLOR color;
    unsigned char pad_1f4[0x1f8 - 0x1f4];
    DWORD flags;
    unsigned char pad_1fc[0x204 - 0x1fc];
    // The ANM file whose script runs.
    AnmLoaded *anm;
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
    unsigned char pad_224[0x288 - 0x224];
    // Added to pos when it is drawn.
    D3DXVECTOR3 posOffset;
    int spriteTime;
    // The size of the characters text drawing uses; 15 from
    // AnmLoaded::StartScriptAtOrigin.
    BYTE fontWidth;
    BYTE fontHeight;
    unsigned char pad_29a[0xa];

    // Resets the sprite's state up to pos, as th08's: a scale of 1,
    // white, the identity matrix, flags 7. Inline; the copy is at
    // 0x4127b0.
    void Initialize()
    {
        memset(this, 0, offsetof(Sprite, pos));
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
class AnmLoaded
{
public:
    // Starts script `script` of this file on `vm`, at the origin with no
    // offset (0x401560).
    void StartScriptAtOrigin(Sprite *vm, int script);
    // Starts `script` on `vm`, or zeroes `vm` if there is no script or
    // the file is still loading (0x4395a0).
    void SetScript(Sprite *vm, void *script);

    // Its slot in SpriteEngine's files.
    int slot;
    void *file;
    int anmCount;
    void *sprites;
    void **scripts;
    void *textures;
    // Nonzero while the textures are still being created.
    int loadStep;
};

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
    // Runs `vm`'s script up to its next wait (0x436f30).
    void Step(Sprite *vm);

    D3DCOLOR modulateColor;
    int modulateColorEnabled;
    unsigned char pad_8[0x4];
    // How many scripts SetScript started.
    int scriptStartCount;
    unsigned char pad_10[0x12880 - 0x10];
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
};
extern SpriteEngine *g_SpriteEngine;
