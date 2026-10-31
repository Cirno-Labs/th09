// The player, one per side ("PlayerInf").

#pragma once

#include "chain.h"
#include "gamemanager.h"
#include "sprite.h"
#include "timer.h"

struct Effect;

// One of the player's hit areas (0x44 bytes), th08's PlayerBombHitbox
// grown. Placeholder name; only the fields known so far.
struct PlayerHitArea
{
    // Zeroes it, then sets unknown34 to 1, as th08's Reset (0x41bda0).
    void Reset();
    // Clears active, as th08's (0x41bd90).
    void Deactivate();

    float x;
    float y;
    // A circle's radius, and what it grows by, as th08's.
    float radius;
    float radiusGrowth;
    // A box's size, as th08's.
    float width;
    float height;
    unsigned char pad_18[0x20 - 0x18];
    // A box's turn, as th08's.
    float angle;
    // Counts down, as th08's.
    int framesLeft;
    // The damage it deals each time, what it dealt so far and the most
    // it may deal, if positive, as th08's.
    int unknown28;
    int hits;
    int maxDamage;
    // It deals damage every unknown34 frames.
    int unknown34;
    // Its shape or kind: 0 and 1 boxes, 2 and 4 circles or boxes as the
    // adders set it.
    int kind;
    BYTE active;
    unsigned char pad_3d[0x40 - 0x3d];
    int unknown40;
};
ASSERT_SIZE(PlayerHitArea, 0x44);

// A part of the player at +0x24 with three timers (0x78 bytes).
// Placeholder name.
struct PlayerUnknown24
{
    // Sets timer5c and timer68 to 0 and clears the low two flags
    // (0x41bdc0).
    void Reset();

    Timer timer0;
    unsigned char pad_c[0x5c - 0xc];
    Timer timer5c;
    Timer timer68;
    DWORD flags;
};
ASSERT_SIZE(PlayerUnknown24, 0x78);

// An entry of PlayerUnknown36c (0x30 bytes). Its implicit constructor
// folds with D3DXVECTOR3's. Placeholder name.
struct PlayerUnknown36cEntry
{
    D3DXVECTOR3 pos;
    // CheckLaserHit's origin.
    D3DXVECTOR3 unknownC;
    D3DXVECTOR3 size;
    // A circle's radius, or 0 for a box, turned by angle unless it is 0.
    float radius;
    float angle;
    void *unknown2c;
};
ASSERT_SIZE(PlayerUnknown36cEntry, 0x30);

// A part of the player at +0x36c: the player and 0x80 entries.
// Placeholder name.
struct PlayerUnknown36c
{
    // Sets the player's hitbox around it and returns the first of the
    // first `count` entries that touches it, or NULL, or NULL while
    // the player's unknown0 is set or unknown1b74 runs (0x41da70).
    PlayerUnknown36cEntry *CheckHits();
    // Adds an entry: a box `size` across at `pos`, unturned (0x40f8e0).
    void Add(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, void *unknown2c);
    // Adds a circle entry of `radius` at `pos`, if one is free. Inline;
    // the copy is at 0x440d30.
    void AddCircle(D3DXVECTOR3 *pos, float radius, void *unknown2c)
    {
        if (this->count < 0x80)
        {
            this->entries[this->count].pos = *pos;
            this->entries[this->count].radius = radius;
            this->entries[this->count].unknown2c = unknown2c;
            this->count++;
        }
    }
    // Add, for a box turned by `angle` about `origin`; used by
    // BulletManager::OnUpdate (0x412830).
    void AddTurned(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, D3DXVECTOR3 *origin, float angle, void *unknown2c);

    Player *player;
    PlayerUnknown36cEntry entries[0x80];
    // How many entries are in use; the player clears it every frame.
    int count;
};
ASSERT_SIZE(PlayerUnknown36c, 0x1808);

// One of the player's options (0x2f4 bytes), the size and timer of
// th08's PlayerOption. Only the fields known so far.
struct PlayerOption
{
    Sprite sprite;
    D3DXVECTOR3 pos;
    unsigned char pad_2b0[0x2e0 - 0x2b0];
    Timer timer;
    unsigned char pad_2ec[0x2f4 - 0x2ec];
};
ASSERT_SIZE(PlayerOption, 0x2f4);

struct Bullet;
struct Enemy;
struct PlayerBullet;
struct ShtFile;

// A character's function kept by the player. Placeholder signature.
typedef int (*PlayerFunc)(Player *player);
// A character's function taking a position, kept by the player.
typedef int (*PlayerPosFunc)(Player *player, D3DXVECTOR3 *pos);
// A character's function that the ECL runs on an enemy.
typedef int (*PlayerEnemyFunc)(Player *player, struct Enemy *enemy);
// A character's function that a combo hit at `pos` runs.
typedef int (*PlayerComboFunc)(struct PlayerUnknown30410 *combo, D3DXVECTOR3 *pos);
struct ShotData;

// A shot's callbacks, as th08's. Under /Gr these are __fastcall.
typedef int (*ShotFireFunc)(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
typedef int (*ShotUpdateFunc)(Player *player, PlayerBullet *bullet);
typedef int (*ShotHitFunc)(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos);

// One of the player's bullets (0x484 bytes), the layout of th08's
// PlayerBullet. Only the fields known so far.
struct PlayerBullet
{
    Sprite sprite;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 trail[0x20];
    D3DXVECTOR3 size;
    // D3DVECTOR, as th08's.
    D3DVECTOR velocity;
    // Character 2's shot fire callback keeps ShotData::offsetY here, and
    // offsetX in velocity.z.
    float unknown448;
    float speed;
    // The direction it flies in.
    float angle;
    Timer timer;
    short damage;
    // 0 unused, 1 flying, 2 hit, as th08's.
    short state;
    // ShotData::type.
    short type;
    unsigned char pad_466[0x468 - 0x466];
    // Set by character 2's shot fire callback: ShotData::option, and
    // 15.
    short unknown468;
    short unknown46a;
    // Its script, from 5.
    short script;
    // Drawn blue while it is hit; cleared when fired, where th08 set
    // its red flag.
    BYTE unknown46e;
    unsigned char pad_46f[0x470 - 0x46f];
    // Cleared when fired, where th08 copied the player's focus.
    short unknown470;
    // Set by character 4's shot update callback.
    BYTE unknown472;
    unsigned char pad_473[0x474 - 0x473];
    // The shot's callbacks, copied when it fires.
    ShotUpdateFunc update;
    ShotUpdateFunc draw;
    ShotHitFunc hit;
    // The shot it was fired from.
    ShotData *shot;
};
ASSERT_SIZE(PlayerBullet, 0x484);

// A part of the player at +0x3031c with a timer (0x1c bytes). Its
// constructor folds with BulletEx's. Placeholder name.
struct PlayerUnknown3031c
{
    Timer timer;
    // The bullet character 2's shot fire callback set the timer for.
    PlayerBullet *bullet;
    unsigned char pad_10[0x1c - 0x10];
};
ASSERT_SIZE(PlayerUnknown3031c, 0x1c);

// A part of the player at +0x30410, which points back to it (0x40
// bytes). Placeholder name.
struct PlayerUnknown30410
{
    // Counts a hit and scores it; `pos` is where (0x41d150).
    void AddCombo(D3DXVECTOR3 *pos, int a, int b, int c, int d);
    // AddCombo, counting unknown38 too (0x40f8b0).
    void AddComboCounted(D3DXVECTOR3 *pos, int a, int b, int c, int d);
    // Sets timer20, count and unknown38 to 0 (0x40f8c0).
    void ResetCombo();
    // Runs the two timers down: when timer20 runs out the count is
    // reset, and when timer14 does the combo ends as in EndCombo
    // (0x41d760).
    void Update();
    // Resets the count, adds bonus to the side's score and clears it
    // (0x41d7e0).
    void EndCombo();

    Player *player;
    // The hits counted, and the most of them.
    int count;
    int maxCount;
    int unknownC;
    // Added to the side's score when the combo ends.
    int bonus;
    Timer timer14;
    Timer timer20;
    unsigned char pad_2c[0x30 - 0x2c];
    // Character 7's unknown30450 takes 5 off it for each attack, down
    // to 0.
    int unknown30;
    // What the character's unknown30450 spends on extra attacks.
    int unknown34;
    int unknown38;
    int unknown3c;
};
ASSERT_SIZE(PlayerUnknown30410, 0x40);

// One of four sprites of the player at +0x30454 (0x2c4 bytes).
// Placeholder name.
struct PlayerUnknown30454
{
    // The sprite's script, less 0x46.
    int script;
    D3DXVECTOR3 pos;
    // Set to (0, -1.5, 0) when started.
    D3DXVECTOR3 velocity;
    // Drawn while set.
    int unknown1c;
    Sprite sprite;
};
ASSERT_SIZE(PlayerUnknown30454, 0x2c4);

// A side's player (0x30f70 bytes). Only the fields known so far.
class Player
{
public:
    // Empty, and defined out of line, as th08's: constructs the members
    // (0x41f070). Their implicit constructors come with it (0x41ee00,
    // 0x41ceb0, 0x41f010, 0x41f030, 0x41ee20, 0x41ee30, 0x41f060).
    Player();

    // Allocates and zeroes a player for side `sideIdx`, keeping `arg` in
    // registerChainArg and `unknown20`, and adds it to the chains (update
    // priority sideIdx + 0xf, draw sideIdx + 0xc and sideIdx + 0xe).
    // Returns it, even if adding fails (0x41f170).
    static Player *RegisterChain(BYTE arg, int sideIdx, int unknown20);
    // The chain callbacks (0x41e900, 0x41eb10, 0x41cdc0, 0x41ee50).
    static int OnUpdate(Player *player);
    static int OnDraw(Player *player);
    static int OnDraw2(Player *player);
    static int AddedCallback(Player *player);
    // Loads each side's character ANM into slot side + 5 (its second
    // one while GameSide::unknown30 is set), unless it is loaded and the
    // character is the last game's. Returns 0, or -1 if one does not
    // load (0x41cde0).
    static int LoadAnms();
    // Frees the player's resources, removes its chain elements and
    // deletes it, if any (0x41ced0).
    static void RemoveChain(Player *player);
    // Resets the player for a new round (0x41ebc0).
    void Reset();
    // Calls the character's unknown30404 with `pos`. Called by the
    // enemies; the copy is at 0x406820.
    int CallUnknown30404(D3DXVECTOR3 *pos);
    // The angle from the player to `pos`, or pi / 2 at the player
    // (0x41bb00).
    float AngleFromPlayer(D3DXVECTOR3 *pos);
    // The angle from `pos` to the player, or pi / 2 at the player
    // (0x41bb70).
    float AngleToPlayer(D3DXVECTOR3 *pos);
    // Raises unknown30388 by `amount` (and character 1 by 0.15 times
    // more), capped at 400, and starts the front's unknown66fc when its
    // hundreds change (0x41bc90).
    void AddUnknown30388(float amount);
    // Whether a box `size` from `pos` each way touches the hitbox
    // (0x41bdf0).
    int CheckHitBox(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);
    // Whether a circle of `radius` at `pos` touches the player, the
    // shot file's hitbox size added (0x41be70).
    int CheckHitCircle(D3DXVECTOR3 *pos, float radius);
    // Whether a box `size` from `pos` each way, turned by `angle` about
    // `origin`, touches the hitbox (1), or, if `canGraze`, comes within
    // 48 of it (2), as th08's CheckLaserHit without the death or the
    // graze (0x41bfc0).
    int CheckLaserHit(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, D3DXVECTOR3 *origin, float angle, int canGraze);
    // Whether a `size` box at `pos` touches grazeTestMin-Max while
    // unknown0 is 0 or 3, as th08's (0x41bee0).
    int TouchesGrazeBox(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);
    // Sets unknown0. Inline; the copy is at 0x403c20.
    void SetUnknown0(int value)
    {
        this->unknown0 = value;
    }
    // Start a circle that clears around the player, growing from 0 by 4,
    // 4 or 3.5 a frame for 0x30, 0x40 or 0x80 frames, with effect 0xd,
    // 0xe or 0xf, and set unknown0 to 3 and unknown303c8 to the frames
    // (0x403c30, 0x403c90, 0x403cf0).
    void StartClearCircle1();
    void StartClearCircle2();
    void StartClearCircle3();
    // Returns unknown0. Out of line, folded with GdiSurface::GetFormat
    // (0x435ec0).
    int GetUnknown0()
    {
        return this->unknown0;
    }
    // Sets registerChainArg (0x41bde0).
    void SetRegisterChainArg(BYTE arg);
    // Takes a free hit area (the last of the free list), or the spare
    // when one is left, resets it and starts it at `pos` with
    // `framesLeft` and `unknown40` (0x41cf30).
    PlayerHitArea *AllocHitArea(D3DXVECTOR3 *pos, int framesLeft, int unknown40);
    // AllocHitArea, then a box (width and height halved) or a circle,
    // with unknown28 for some and kind 1, 0, 4 or 2 (0x41cfa0, 0x41cfe0,
    // 0x41d010, 0x41d050, 0x41d090, 0x41d0d0, 0x41d110).
    PlayerHitArea *AddHitBox(D3DXVECTOR3 *pos, float width, float height, int framesLeft, int unknown40);
    PlayerHitArea *AddHitCircle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int framesLeft, int unknown40);
    PlayerHitArea *AddDamageBox(D3DXVECTOR3 *pos, float width, float height, int unknown28, int framesLeft, int unknown40);
    PlayerHitArea *AddDamageCircle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int unknown28, int framesLeft, int unknown40);
    PlayerHitArea *AddKind4Box(D3DXVECTOR3 *pos, float width, float height, int unknown28, int framesLeft, int unknown40);
    PlayerHitArea *AddKind4Circle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int unknown28, int framesLeft, int unknown40);
    PlayerHitArea *AddKind2Circle(D3DXVECTOR3 *pos, float radius, float radiusGrowth, int unknown28, int framesLeft, int unknown40);
    // The damage the bullets and the hit areas deal to an enemy with a
    // `size` box at `pos`, as th08's: the bullets' damage is stored in
    // `damageTaken`, and the kind 4 areas' damage added to `kind4Damage`.
    // Clears `hitByBomb` and nothing sets it. The bullets that hit stop
    // and turn to their hit script (0x41fcd0).
    int CalcDamage(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, int *damageTaken, int *hitByBomb, int *kind4Damage);
    // Lowers unknown30388 by `amount`, down to 1 (0x41f310).
    void SubUnknown30388(float amount);
    // Moves the bullets and runs their update callbacks, freeing those
    // that leave the play area or end, unless the side is frozen
    // (flags bit 0), as th08's UpdateBullets (0x41f580).
    void UpdateBullets();
    // Draws the flying bullets (state 1) as DrawHitBullets does, through
    // their draw callback if they have one, as th08's DrawBullets
    // (0x41f680).
    void DrawBullets();
    // Tests the hit areas against a bullet (0x41dff0).
    int CheckHitAreas(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, Bullet *bullet);
    // Returns 2 if the hit areas take the bullet, else whether a `size`
    // box at `pos`, grown by 60 each way, touches grazeMin-Max, as
    // th08's CheckGraze without its state test or Graze call
    // (0x41e350).
    int CheckGraze(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, Bullet *bullet);
    // Starts the first free entry of unknown30454 at `pos` with script
    // 0x46 + `script` of the side's bullet ANM, unless the front's
    // unknown1095c is set (0x41db90).
    void AddUnknown30454(int script, D3DXVECTOR3 *pos);
    // Draws the sprites of unknown30454 that are set at their positions
    // (0x41df80).
    void DrawUnknown30454();
    // Draws the bullets that hit (state 2) at their positions on the
    // side's screen, turned to their angle when they rotate, blue while
    // unknown46e is set, as th08's DrawHitBullets (0x41f760).
    void DrawHitBullets();

    // OnDraw draws only unknown30454 while it is 5.
    int unknown0;
    // RegisterChain's first argument.
    BYTE registerChainArg;
    unsigned char pad_5[0x8 - 0x5];
    // The side, 0 or 1.
    int sideIdx;
    GameSide *side;
    GameSide *otherSide;
    ChainElem *calcElem;
    ChainElem *drawElem;
    ChainElem *drawElem2;
    int unknown20;
    PlayerUnknown24 unknown24;
    // Set by AddedCallback: the player, 1, 1.
    Player *unknown9c;
    int unknownA0;
    int unknownA4;
    // Draws the player whatever the front's unknown1095c and
    // unknown10960.
    int unknownA8;
    // Set by AddedCallback: 2, 0, 0, 0.
    int unknownAc;
    int unknownB0;
    int unknownB4;
    int unknownB8;
    // The character's ANM, in slot sideIdx + 5.
    AnmLoaded *anm;
    Sprite sprite;
    unsigned char pad_364[0x368 - 0x364];
    // An effect whose unknown7c is the radius the character's
    // unknown30404 tests.
    Effect *unknown368;
    PlayerUnknown36c unknown36c;
    Timer unknown1b74;
    unsigned char pad_1b80[0x1b88 - 0x1b80];
    D3DXVECTOR3 pos;
    unsigned char pad_1b94[0x1ba0 - 0x1b94];
    // The player's last positions, as th08's.
    D3DXVECTOR3 posHistory[0x10];
    // The hitbox, the graze box and the box TouchesGrazeBox tests, as
    // th08's.
    D3DXVECTOR3 hitboxMin;
    D3DXVECTOR3 hitboxMax;
    D3DXVECTOR3 grazeMin;
    D3DXVECTOR3 grazeMax;
    D3DXVECTOR3 grazeTestMin;
    D3DXVECTOR3 grazeTestMax;
    // Half the shot file's sizes, square, with a z of 5, as th08's.
    D3DXVECTOR3 hitboxSize;
    D3DXVECTOR3 grazeSize;
    D3DXVECTOR3 unknown1cc0;
    // Below 0 or above 0 by the way the player moves across, as
    // character 10's effect update callback tells.
    float unknown1ccc;
    unsigned char pad_1cd0[0x1cdc - 0x1cd0];
    // Character 3's shot callback sets both to 0.5.
    float unknown1cdc;
    float unknown1ce0;
    unsigned char pad_1ce4[0x1cec - 0x1ce4];
    PlayerOption options[4];
    // The hit areas; the last is the spare AllocHitArea hands out when
    // none is free.
    PlayerHitArea hitAreas[0x201];
    // The hit areas in use and the free ones, and how many of each.
    PlayerHitArea *activeHitAreas[0x202];
    PlayerHitArea *freeHitAreas[0x202];
    int activeHitAreaCount;
    int freeHitAreaCount;
    unsigned char pad_c118[0xc11c - 0xc118];
    PlayerBullet bullets[0x80];
    PlayerUnknown3031c unknown3031c;
    // The character's shot file (plXX.sht).
    ShtFile *shtFile;
    unsigned char pad_3033c[0x30354 - 0x3033c];
    // Counted up by character 1's shot hit callback.
    BYTE unknown30354;
    unsigned char pad_30355[0x30364 - 0x30355];
    // Where character 12's shot fire callback aims.
    D3DXVECTOR3 unknown30364;
    unsigned char pad_30370[0x3037c - 0x30370];
    // An enemy, forgotten when it despawns.
    Enemy *unknown3037c;
    unsigned char pad_30380[0x30388 - 0x30380];
    // Lowered by SubUnknown30388, down to 1, and raised by
    // AddUnknown30388, up to 400.
    float unknown30388;
    Timer unknown3038c;
    Timer unknown30398;
    Timer unknown303a4;
    Timer unknown303b0;
    Timer unknown303bc;
    Timer unknown303c8;
    Timer unknown303d4;
    unsigned char pad_303e0[0x303f0 - 0x303e0];
    // The character's values from the tables at 0x4a1bd0, 0x4a1b90,
    // 0x4a1b50, 0x4a1b10 and 0x4a1ad0.
    int unknown303f0;
    int unknown303f4;
    int unknown303f8;
    int unknown303fc;
    int unknown30400;
    // The character's functions from the tables at 0x4a1a50 and
    // 0x4a1a90. The first takes a position, from CallUnknown30404.
    PlayerPosFunc unknown30404;
    PlayerEnemyFunc unknown30408;
    unsigned char pad_3040c[0x30410 - 0x3040c];
    PlayerUnknown30410 unknown30410;
    // The character's function from the table at 0x4a19d0.
    PlayerComboFunc unknown30450;
    PlayerUnknown30454 unknown30454[4];
    unsigned char pad_30f64[0x30f70 - 0x30f64];
};
ASSERT_SIZE(Player, 0x30f70);

// One shot of a shot file, th08's ShotData with the frame it fires
// on in place of its interval. Only the fields known so far.
struct ShotData
{
    // The frame it fires on.
    short frame;
    unsigned char pad_2[0x4 - 0x2];
    // Where it starts, from the player or its option.
    float offsetX;
    float offsetY;
    float sizeX;
    float sizeY;
    float angle;
    float speed;
    short damage;
    unsigned char pad_1e[0x20 - 0x1e];
    // 0 for the player, else the option, from 1.
    short option;
    // PlayerBullet::type.
    short type;
    // Its script, from 5, and its sound or -1.
    short script;
    short sound;
    // Fires it; TryFireShot when NULL.
    ShotFireFunc fire;
    // Copied into the bullet it fires.
    ShotUpdateFunc update;
    ShotUpdateFunc draw;
    ShotHitFunc hit;
};
ASSERT_SIZE(ShotData, 0x38);

// A table of shots, ending with a frame below 0. Only the fields known
// so far.
struct ShotTable
{
    ShotData *shots;
    unsigned char pad_4[0x8 - 0x4];
};
ASSERT_SIZE(ShotTable, 0x8);

// A character's shot file (plXX.sht), th08's ShotTypeData
// rearranged. Only the fields known so far.
struct ShtFile
{
    unsigned char pad_0[0x2];
    // How many entries tables has, as th08's.
    WORD tableCount;
    // The player's hitbox and graze box, both square, as th08's.
    float hitboxSize;
    float grazeSize;
    unsigned char pad_c[0x10 - 0xc];
    float unknown10;
    // The movement speeds, straight and diagonal, as th08's; the
    // diagonal ones are set when the file loads.
    float speed;
    float speedFocused;
    float speedDiagonal;
    float speedDiagonalFocused;
    unsigned char pad_24[0x42c - 0x24];
    ShotTable tables[1];
};

// Fills `bullet` with shot `sd` and starts it, as th08's (0x41f350).
void FireShot(Player *player, PlayerBullet *bullet, ShotData *sd);
// Fires `sd` into `bullet` if `frame` is its frame, returning 1 if it
// did, as th08's, which tested frame % interval against phase
// (0x41f490).
int TryFireShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd);
// Fires the shots of table `table` of the shot file into the free
// bullets, as th08's FireShots, which picked the table itself
// (0x41f4c0).
void FireShots(Player *player, int frame, int table);

// Each character's files: its shot file, ANM, ECL, MSG, match MSG and
// second ANM (0x4a1238).
extern char *g_CharacterFiles[][6];

// Each side's shot file, kept while the side keeps its character
// (0x4a8160).
extern ShtFile *g_ShtFiles[2];

// The shot callbacks a shot file's indices name (0x4a1c10, 0x4a1c34,
// 0x4a1c54, 0x4a1c64).
extern ShotFireFunc g_ShotFireFuncs[9];
extern ShotUpdateFunc g_ShotUpdateFuncs[8];
extern ShotUpdateFunc g_ShotDrawFuncs[4];
extern ShotHitFunc g_ShotHitFuncs[4];

// Each character's values and functions the player keeps (0x4a19d0,
// 0x4a1a50, 0x4a1a90, 0x4a1ad0, 0x4a1b10, 0x4a1b50, 0x4a1b90,
// 0x4a1bd0).
extern PlayerComboFunc g_CharacterFuncs9d0[16];
extern PlayerPosFunc g_CharacterFuncsA50[16];
extern PlayerEnemyFunc g_CharacterFuncsA90[16];
extern int g_CharacterValuesAd0[16];
extern int g_CharacterValuesB10[16];
extern int g_CharacterValuesB50[16];
extern int g_CharacterValuesB90[16];
extern int g_CharacterValuesBd0[16];
