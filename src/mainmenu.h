// The title menu. Only the fields known so far.

#pragma once

#include "chain.h"
#include "common.h"
#include "gamemanager.h"
#include "network.h"
#include "replay.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

class MainMenu;

// The help lines of the title's eight entries (0x4a1e3c).
extern char *g_TitleHelpTexts[8];
// The demo replays the title plays in turn (0x4a1ce8).
extern char *g_DemoReplayPaths[3];
// The characters a replay's name is typed with, as th08's (0x4a1cf4).
extern char *g_NameAlphabet;
// The replay menu's names of the characters (padded to 8), the
// difficulties, the modes 0 and 1 and the four kinds of match of mode 2
// (0x4a1cf8, 0x4a1d3c, 0x4a1d50, 0x4a1d58).
extern char *g_ReplayMenuCharacterNames[17];
extern char *g_ReplayMenuDifficultyNames[5];
extern char *g_ReplayMenuModeNames[2];
extern char *g_ReplayMenuMatchNames[4];
// "1st" to "5th", for the high score tables (0x4a1d68).
extern char *g_ScoreRankNames[5];

// The help lines of the options' nine entries (0x4a1ddc).
extern char *g_OptionsHelpTexts[9];
// The help lines of the key config's ten entries (0x4a1e04), and the
// pad button the key config last saw pressed (0x4a1e00).
extern char *g_KeyConfigHelpTexts[10];
extern short g_LastPressedKey;
// The comment the music room shows for a tune not heard yet (0x4a1dbc).
extern char *g_MusicNotHeardComment[8];
// The characters of state 8's character select against the CPU, in
// its order (0x4a1d8c).
extern char g_VsCpuCharacters[16];
// The characters of state 8's character select, in its order
// (0x4a1e2c).
extern char g_VsCharacters[16];
// The characters of states 3 and 5's character select, in its order
// (0x4a1d7c; the last two are padding).
extern char g_StoryCharacters[16];
// The characters in the order the character select lists them
// (0x4a1d9c), and the open ones of them, then -1s, as UpdateState16
// found them (0x4ac8c4).
extern char g_CharacterOrder[16];
extern char g_OpenCharacters[16];
// The characters in the order the high score tables list them
// (0x4a1dac).
extern char g_ScoreCharacters[16];
// How far into the code that opens every entry of the menus' lists the
// high score tables are (0x4ac8d4).
extern int g_CheatCodeProgress;

// Cleared by the title (0x4ac884).
extern int g_Unknown4ac884;

// A tune in the music room's list, as th08's.
struct MusicTune
{
    // Zeroes the tune. Inline in th08; here its copy starts the menu's
    // object (0x422f00).
    MusicTune();

    // The MIDI file.
    char path[0x40];
    char title[0x42];
    // The comment shown for the tune.
    char comment[8][0x42];
};
ASSERT_SIZE(MusicTune, 0x292);

class MainMenu
{
public:
    // Constructs the replay headers, the tunes, the sprites and the
    // config, then zeroes the menu, as th08's. Inline there; here the
    // copy is out of line (0x429d83).
    MainMenu();

    // Queues sound effect `idx` with `arg` and plays it at once
    // (0x422f17).
    void PlaySoundNow(int idx, int arg);
    // Moves to state `state`, remembering the one it leaves in
    // prevState, and resets the frame counters, as th08's. Inline, but
    // this object calls the copy at 0x422f39, and what it does to the
    // registers is not known to the callers.
    void SetState(int state)
    {
        this->prevState = this->state;
        this->state = state;
        this->stateFrames = 0;
        this->inputLockout = 0;
        this->subState = 0;
        this->subMenuIdleFrames = 0;
    }
    // Gives every button of `mapping` assigned `button` the value
    // `replacement`, as th08's. The last argument is unused (0x422f67).
    void ReassignButton(ControllerMapping *mapping, short button, short replacement, int unused);
    // Shows `value` as two digits on `vm` and the sprite after it, or
    // hides both when it is negative, as th08's. Returns 0 (0x422fd1).
    int ShowButtonDigits(Sprite *vm, short value);
    // Moves cursors[idx] by `delta` among `count` entries, wrapping,
    // until it is on one that g_PlayRecord's openEntries marks open; entries[i] is
    // the entry at i. Not while unknownA8 of g_Network is set
    // (0x42304d; 0x4230a6 and 0x4230ff with openEntries2 and 3).
    void SkipClosedEntries(int idx, int delta, char *entries, int count);
    void SkipClosedEntries2(int idx, int delta, char *entries, int count);
    void SkipClosedEntries3(int idx, int delta, char *entries, int count);
    // Shows `left` and `right` in halves on the five sprites from 142
    // and the five from 148: 0xc3 for a full one, 0xc4 for a half, 0xc5
    // for an empty one. Not called (0x423182).
    void ShowHalves(int left, int right);
    // What OnDraw draws on top in states 11, 15, 13 and 14, and 12
    // (0x4234f6, 0x4239f6, 0x423d16, 0x4244a9). State 15's lists the
    // replay slots (subState 2), then moves the chosen one up and shows
    // the name being typed (3), with the alphabet to type it with (4).
    // State 11's lists the replays (subState 1), then moves the chosen
    // one up and shows its mode and its rounds' characters (2).
    // States 13 and 14 show scoreCharacter's five high score tables,
    // fading in; state 14 lights only the difficulty's, marks the new
    // entry at scoreRank and shows the alphabet to name it with.
    int DrawState11();
    int DrawState15();
    int DrawState13();
    int DrawState12();
    // States 3, 5 and 7 draw nothing more, but OnDraw's switch has
    // cases for them: they call something that compiles to nothing.
    int DrawNothing()
    {
        return 1;
    }

    // Moves cursor by up and down among `count` entries, wrapping, as
    // th08's MoveMenuCursor; returns -1, 1, or 0 when it did not move
    // (0x42457a).
    int MoveMenuCursor(int count);
    // The same with left and right (0x424601).
    int MoveMenuCursorLR(int count);
    // The same with all four directions, left and right first
    // (0x424671).
    int MoveCursor4(int count);
    // MoveMenuCursorLR, up and down as left and right, on cursors[idx] with
    // player `idx`'s input (0x42474a).
    int MovePlayerCursor(int idx, int count);
    // The same on cursors[idx] with player `player`'s input (0x4247f1).
    int MovePlayerCursor2(int player, int idx, int count);
    // Draws the menu's sprites, then the state's own, as th08's
    // (0x424898).
    static int OnDraw(MainMenu *menu);
    // Loads the title's ANM files and sets up its sprites, run in the
    // sub thread, as th08's (0x4249e1).
    static void LoadingThread(MainMenu *menu);
    // Frees the files and sprites the title scene loaded, as th08's
    // (0x424d4b).
    int ReleaseTitleScene();
    // Releases the title's ANM files and surface, unregisters the menu
    // and deletes it, as th08's (0x424d9f).
    static int DeletedCallback(MainMenu *menu);
    // Sets up the title as th08's, then loads it in LoadingThread in the
    // sub thread. Returns 0 (0x424bf7).
    int InitTitleScene();
    // The update element's added callback: InitTitleScene (0x424d46).
    static int AddedCallback(MainMenu *menu);
    // Shows sprites first to first + count - 1 dimmed, the next frame
    // of their ANM, with interrupt 8, and the one at cursor lit with
    // interrupt 7 (0x424dff).
    void SelectEntry(int cursor, int first, int count);
    // Shows frame `frame` of the entry sprite `idx`, with interrupt 7
    // (0x424e95).
    void ShowEntryFrame(int frame, int idx);
    // SelectEntry for every stride-th sprite, stepping each at once
    // (0x424edd).
    void SelectEntryStep(int cursor, int first, int count, int stride);
    // SelectEntry for every stride-th sprite with interrupts 15 and 14
    // (0x424fb0), and with 18 and 17 (0x42505f).
    void SelectEntry2(int cursor, int first, int count, int stride);
    void SelectEntry3(int cursor, int first, int count, int stride);
    // SelectEntry's interrupts without changing the sprites (0x42510e).
    void SelectEntryInterrupts(int cursor, int first, int count);
    // Sends interrupt 9 to the sprites ShowHalves sets and those next
    // to them, then shows the halves (0x425163).
    void FlashHalves(int left, int right);
    // Sends sprite 153 + idx interrupt 9 if `on`, else 10 (0x42520b).
    void FlashSprite(int idx, int on);
    // Moves player `player`'s cursor sprite to the row of entry
    // `selected` among the open ones of `entries`, and lights the rows
    // of `selected` and `selected2`. Open is by g_PlayRecord's
    // openEntries in state 8, where network play opens all, openEntries2
    // in state 3 and openEntries3 otherwise, except for the lighting,
    // which always goes by openEntries (0x425233).
    void MoveCharacterCursor(int player, int selected, int selected2, char *entries);
    // The same for sprites 189 and 190 and one selection; the last two
    // entries are always shown (0x425390).
    void MoveCharacterCursor2(int player, int selected, char *entries);

    // The states' update functions, by the first state each runs in.
    // UpdateState2 also runs states 4 and 7, UpdateState3 state 5, and
    // state 8 runs UpdateState8b instead of UpdateState8 when
    // g_GameManager's unknown114 is 1 to 3. UpdateState13 runs the high
    // score tables: left and right pick the character among the open
    // ones, a cancel goes back to state 1, and a code of ten buttons
    // opens every entry. UpdateState14 puts g_Hscr in the tables and
    // has its name typed in, starting from g_Lsnm's; ending it saves
    // the scores and goes to state 15 (mode 2 goes there at once).
    // UpdateState11 reads the replays (th9_01 to 25 and up to 25 user
    // ones) and lets one and its round be chosen, then plays it.
    // UpdateState3 is the story character select (g_StoryCharacters,
    // opened by openEntries2, or openEntries3 on difficulty 4): cursors[0]
    // and [1] pick the two characters and cursors[2] and [3] mark them
    // chosen; while button 4 is held, unknown11b84 can be changed. It
    // starts the game in scene 2, or goes back to state 2, 4 or 7.
    // UpdateState8 is the versus character select of two players
    // (g_VsCharacters, opened by openEntries or network play), each with
    // their own input, cursors[0] and [1], and chosen marks cursors[2]
    // and [3]; once both chose it goes to state 16.
    // UpdateState8b is the same against the CPU (unknown114 1 to 3):
    // the one player picks their character, then the opponent's.
    // UpdateState9 is the options, th08's UpdateOptions: unknownAc, the
    // colour depth, the music, the two volumes and the window, the
    // defaults, the key config (state 10) and leaving, which returns 5
    // when the colour depth or the window changed.
    // UpdateState10 is the key config, th08's UpdateKeyConfig for the
    // player unknownC920: the pad, unknownB4 and shoot, bomb, focus,
    // skip and menu, which only the joysticks set; saving copies the
    // mappings to the config and the inputs.
    // UpdateState12 is the music room, th08's music screen: it reads
    // musiccmt.txt into tunes, lists ten tunes from listTop, shows the
    // playing one's comment a line every two frames and plays, pauses
    // or fades out the tune.
    // UpdateState15 asks whether to save the replay, lists the slots
    // th9_01 to 25, asks before overwriting one and has the name typed
    // in; network play saves to the first free th9_udnXXX at once.
    // UpdateState16 is the character select of states 3 and 5 and of
    // mode 2's matches, listing the open characters of
    // g_CharacterOrder; it starts the game in scene 2, or goes back to
    // state 8.
    int UpdateState3();
    int UpdateState8b();
    int UpdateState16();
    int UpdateState13();
    int UpdateState11();
    int UpdateState12();
    int UpdateState9();
    int UpdateState10();
    int UpdateState8();
    int UpdateState14();
    int UpdateState15();
    int UpdateState1();
    int UpdateState2();
    int UpdateState6();
    // SelectEntry on the entries at entryFirst, with cursor (0x4276d7).
    void SelectCursorEntry();
    // The update element's callback, as th08's: does nothing while the
    // title loads (returns 4 if that failed, else 1), otherwise runs the
    // state's function, then steps the sprites (0x42a9cb).
    static int OnUpdate(MainMenu *menu);

    int cursor;
    // Where the next character of replayName goes.
    int replayNameCursor;
    // The character whose high scores state 13 or 14 shows, and the
    // place of the new entry in state 14.
    int scoreCharacter;
    int scoreRank;
    int cursors[5];
    // The entry whose help UpdateState1 last showed.
    int prevCursor;
    int subState;
    // Counts the frames in the current state.
    int stateFrames;
    unsigned char pad_30[0x84 - 0x30];
    // The state before the current one.
    int prevState;
    int unknown88;
    // The name typed in for the replay being saved.
    char replayName[9];
    unsigned char pad_95[0x28c - 0x95];
    // The paths of the replay files, as th08's: "./replay/th9_udXXXX.rpy"
    // for those from 25 on.
    char replayPaths[50][0x200];
    unsigned char pad_668c[0x681c - 0x668c];
    // Files ReleaseTitleScene frees.
    void *unknown681c[50];
    // The headers of the replay files.
    ReplayHeader replays[50];
    void *loadedReplay;
    // Set to 0 and 50 once UpdateState11 has read the replays.
    int unknownC900;
    int replayCount;
    // The replay slot being saved to.
    int replaySlot;
    unsigned char pad_c90c[0xc910 - 0xc90c];
    int subMenuIdleFrames;
    unsigned char pad_c914[0xc918 - 0xc914];
    int unknownC918;
    unsigned char pad_c91c[0xc920 - 0xc91c];
    int unknownC920;
    unsigned char pad_c924[0xc934 - 0xc924];
    // The music room's tunes.
    MusicTune tunes[32];
    // The music room's tune playing, how many tunes musiccmt.txt lists,
    // the first of the ten shown, and whether the tune is paused.
    int playingTune;
    int tuneCount;
    int listTop;
    int musicPaused;
    // Go to GameManager::initialLives.
    int unknown11b84;
    int unknown11b88;
    // Set while UpdateState3 lets left and right change the two
    // values above.
    int unknown11b8c;
    // The title's ANM file, and the result screen's text.
    AnmLoaded *anm;
    AnmLoaded *resultTextAnm;
    // spriteCount sprites, drawn by OnDraw.
    Sprite *sprites;
    // Drawn after them when set.
    Sprite *helpSprite;
    // Sprites of textAnm's script 5, as th08's.
    Sprite helpSprites[14];
    Sprite textSprites[21];
    // Sprites 0x15 to 0x22 of textAnm, as th08's card details.
    Sprite cardDetailSprites[14];
    // The music room's comment, a line each.
    Sprite commentSprites[8];
    int spriteCount;
    int state;
    // 1 makes InitTitleScene start in state 14.
    int unknown1b22c;
    int inputLockout;
    // The frames without input on the title; at 1500 a demo plays.
    int idleFrames;
    // While nonzero, OnDraw draws nothing: 1 while LoadingThread loads,
    // 2 if it failed.
    int loadingState;
    int unknown1b23c;
    // The entries SelectCursorEntry lights: the first sprite and the
    // count.
    int entryFirst;
    int entryCount;
    ChainElem *calcElem;
    ChainElem *drawElem;
    // The key config's copy of the config's mappings.
    ControllerMapping mappings[2][3];
    GameConfig config;
};
ASSERT_SIZE(MainMenu, 0x1b388);

// Creates the title menu and registers its chain elements, as th08's.
// `arg` goes to unknown1b22c. Returns 0, or -1 if the update element
// could not be added (0x42ab13).
int SetUpTitleScene(int arg);
