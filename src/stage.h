// The stage: its background and scrolling, one per side.

#pragma once

#include "chain.h"
#include "sprite.h"
#include "timer.h"

struct GameSide;

// An instruction of the stage script: at `frame`, `opcode` acts on
// `args`, as th08's.
struct StageInstr
{
    int frame;
    short opcode;
    short unusedShort06;
    D3DXVECTOR3 args;
};

// A run of quads in the stage data, ended by one with a negative type,
// as th08's.
struct StageQuad
{
    short type;
    // The offset of the next quad.
    short size;
    short script;
    // Set by Load: the quad's Sprite in Stage::sprites.
    short spriteIndex;
    D3DXVECTOR3 pos;
    float width;
    float height;
};

// An object of the stage data: its quads start at +0x1c. As th08's.
struct StageObject
{
    BYTE pad_00[0x2];
    // The background layer it is drawn on.
    char layer;
    // Bit 0: its sprites still run scripts. Bit 1: DrawLayer drew it.
    char flags;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 size;
    StageQuad quads[1];
};

// A placement of an object; a negative `id` ends the list.
struct StageInstance
{
    short id;
    short unusedShort02;
    D3DXVECTOR3 pos;
};

// The start of the stage data file, as th08's.
struct StageData
{
    short objectCount;
    short quadCount;
    int instancesOffset;
    int scriptOffset;
    DWORD unusedDword0c;
    char name[0x80];
    char songNames[4][0x80];
    char songPaths[4][0x80];
    // Then, at 0x490, the objects' offsets.
};

// D3DRS_FOGSTART, D3DRS_FOGEND and D3DRS_FOGCOLOR, as th08's.
struct StageFog
{
    float nearPlane;
    float farPlane;
    D3DCOLOR color;
};

// A camera of the stage script, as th08's, laid out as App's cameras.
// Its vectors are D3DVECTORs: the stage's constructor builds none.
struct StageCamera
{
    D3DVECTOR eye;
    D3DVECTOR at;
    D3DVECTOR up;
    D3DVECTOR dir;
    D3DVECTOR cameraRight;
    D3DVECTOR offset;
    float fov;
    unsigned char pad_4c[0xf0 - 0x4c];
};

// A side's stage ("BGScrollInf", 0x65c4 bytes). Only the fields known so
// far.
class Stage
{
public:
    // Zeroes the stage (0x403a40).
    Stage();

    // Removes the stage's chain elements and deletes it, if any
    // (0x402530).
    static void RemoveChain(Stage *stage);
    // Resets the stage for a new round (0x4011c0).
    void Reset();
    // The update chain callback and its added and deleted callbacks
    // (0x402350, 0x403830, 0x4024d0). DeletedCallback frees the stage's
    // ANM, slot 4, if resources are freed, and its two buffers.
    static int OnUpdate(Stage *stage);
    static int AddedCallback(Stage *stage);
    static int DeletedCallback(Stage *stage);
    // The two draw chain callbacks (0x4033e0, 0x403680).
    static int OnDraw(Stage *stage);
    static int OnDraw2(Stage *stage);
    // Creates side `sideIdx`'s stage and adds its update element
    // (priority 8 + `sideIdx`) and draw elements (6 + `sideIdx` and 8 +
    // `sideIdx`). Returns it, or NULL if adding fails (0x403ae0).
    static Stage *RegisterChain(int sideIdx);
    // Draws background layer `layer` (0x4026c0).
    void DrawLayer(int layer);
    // Jumps the script to the label waitLabel, if set, as th08's
    // OnUpdate began (0x4018a0).
    void JumpToWaitLabel();
    // Runs the stage script up to the current frame, as th08's OnUpdate,
    // on App's camera of the side. Then fades the cameras and the fog
    // and shakes the camera (0x4018f0).
    void RunScript();
    // Runs one frame of the scripts of each animated object's sprites,
    // as th08's, without colorScaleSprite (0x4011d0). Returns 0.
    int StepObjects();
    // Sets spellCardState to 1 and clears spellCardFrames. Called by the ECL
    // interpreter; the copy is at 0x4067b0.
    void StartSpellCard();

    ChainElem *calcElem;
    ChainElem *drawElem;
    // A second draw element.
    ChainElem *drawElem2;
    int sideIdx;
    // The side, and the other one.
    GameSide *side;
    GameSide *otherSide;
    // The label opcode 31 marks that the script jumps to, as th08's.
    int waitLabel;
    // OnUpdate tints the stage by tintFadeMode (1 red, 2 dark blue)
    // until tintFadeFrames runs out.
    int tintFadeMode;
    int tintFadeFrames;
    // One Sprite per object sprite in the data. Freed by
    // DeletedCallback.
    Sprite *sprites;
    StageInstr *script;
    // Drawn under the layers while spellCardState is at most 1; the
    // first is placed by OnDraw.
    Sprite backgroundLayer0;
    Sprite backgroundLayer1;
    Sprite backgroundLayer2;
    AnmLoaded *anm;
    unsigned char pad_81c[0x820 - 0x81c];
    // Freed by DeletedCallback.
    StageData *data;
    int quadCount;
    int objectCount;
    StageObject **objects;
    StageInstance *instances;
    // The cameras the fades run between, and the Hermite tangents, as
    // th08's.
    StageCamera cameraFrom;
    StageCamera cameraTo;
    StageCamera cameraFadeTarget;
    StageCamera cameraFadeStart;
    // The fades of eye, at, up, fov and the shake, as th08's.
    int cameraFadeFrames[5];
    Timer cameraFadeTimer[5];
    int cameraFadeMode[5];
    Timer scriptTime;
    int scriptIndex;
    // Cleared by RegisterChain, counted by OnUpdate.
    int unknownC68;
    unsigned char pad_c6c[0xc70 - 0xc6c];
    D3DVECTOR cameraPos;
    // OnDraw clears to the fog color if this is opaque, draws the fog
    // color over the arena if it is otherwise nonzero.
    D3DCOLOR clearColor;
    unsigned char pad_c80[0xc84 - 0xc80];
    Timer unknownC84;
    unsigned char pad_c90[0xc94 - 0xc90];
    StageFog fog;
    StageFog fogStart;
    StageFog fogTarget;
    int fogFadeFrames;
    Timer fogFadeTime;
    // Set by SetDefaultRenderStates, as th08's.
    unsigned char fogColorChanged;
    unsigned char pad_cc9[0xccc - 0xcc9];
    // Cleared by Reset. The layers are drawn while it is at most 1.
    int spellCardState;
    // Counted by OnUpdate while spellCardState is at least 1.
    int spellCardFrames;
    // OnDraw clears the target to black once if set.
    int clearScreenOnce;
    // Drawn by OnDraw2 once spellCardState is 1, at their posOffset in
    // the arena.
    int dialogueSpriteCount;
    unsigned char pad_cdc[0xce0 - 0xcdc];
    Sprite dialogueSprites[0x20];
    Sprite unknown6160;
    // Called by OnDraw2 once spellCardState is 1, if set.
    void (*spellCardDrawCallback)();
    // The script's key positions and their frames, as th08's.
    D3DVECTOR nextKeyPos;
    int nextKeyFrame;
    D3DVECTOR keyPos;
    int keyFrame;
    // Set by opcode 4, cleared by opcode 5, as th08's.
    BYTE cameraOffsetChanged;
    unsigned char pad_6429[0x642c - 0x6429];
    // OnDraw modulates sprites by it if its alpha is nonzero.
    union
    {
        D3DCOLOR color;
        struct
        {
            BYTE b;
            BYTE g;
            BYTE r;
            BYTE a;
        };
    } tintColor;
    // OnDraw stops modulating sprites unless set.
    // OnDraw2 clears it.
    int keepColorModulation;
    unsigned char pad_6434[0x6438 - 0x6434];
    // The camera shake opcode 33 starts, as th08's.
    BYTE shakeMode;
    unsigned char pad_6439[0x6440 - 0x6439];
    // Cleared by OnDraw2.
    int unknown6440;
    D3DXVECTOR3 unknown6444[0x20];
};
ASSERT_SIZE(Stage, 0x65c4);
