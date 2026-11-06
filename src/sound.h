// The sound player and the game's music and sound tables, as th08's.

#pragma once

#include <windows.h>

#include "common.h"
#include "dsutil.h"
#include "unsorted.h"

// A piece of music.
struct BgmInfo
{
    int id;
    // The MIDI file; the wave file has the same name.
    char *path;
    int unknown08;
    int unknown0c;
};
// The game's music, at 0x4a21d0.
extern BgmInfo g_BgmInfos[19];
// The unknown08 of the piece PlayMusic played last (0x4dc690); the
// gui reads it.
extern int g_BgmUnknown08;

// A command PlayQueuedSounds carries out over several frames, for the
// streaming wave music, as th08's.
struct SoundCommand
{
    // 0 ends the queue.
    int type;
    int arg;
    // How far the command has got.
    int stage;
    char path[0x100];
};
ASSERT_SIZE(SoundCommand, 0x10c);

// An entry of thbgm.fmt: where a track is in thbgm.dat and its format,
// as th08's.
struct BgmFormat
{
    // The track's file name; an empty one ends the list.
    char name[0x10];
    DWORD offset;
    DWORD size;
    // CWaveFile::ResetFile limits the track to `loopEnd` bytes and, when
    // looping, skips its first `loopStart`.
    int loopStart;
    int loopEnd;
    WAVEFORMATEX format;
};
ASSERT_SIZE(BgmFormat, 0x34);

// A sound effect: the loaded file it plays, its volume and its priority.
struct SoundInfo
{
    int buffer;
    short volume;
    short unk_6;
};
// 0x36 sound effects, at 0x4a1f80 (th08 0x2e).
extern SoundInfo g_SoundInfos[0x36];
// The sound files LoadSounds loads, by index: 0x27 (th08 0x24), at
// 0x4a2130.
extern char *g_SoundPaths[0x27];

// Plays the sound effects and music, as th08's (at 0x4dc698).
class SoundPlayer
{
public:
    // Zeroes everything; no sound is loaded (0x43e470).
    SoundPlayer();
    // Creates the DirectSound objects for `hwnd`. Returns 0, or -1 on
    // failure (0x43e500).
    int Initialize(HWND hwnd);
    // Releases the sounds and DirectSound (0x43eeb0).
    int Release();
    // Stops the wave music: ends the notification thread and releases
    // the stream (0x43ebc0).
    void StopBgm();
    // The index in bgmFormat of the track named as the file name of
    // `path`, or 0 if there is none (0x43e1f0).
    int FindTrackIndex(char *path);
    // Loads the wave file at `path` into soundBuffers[`index`]. Returns
    // 0, also when there is no sound, or -1 on failure (0x43e770).
    int LoadSound(int index, char *path);
    // Loads the track list at `path` (thbgm.fmt) into bgmFormat. Returns
    // 0 or -1 (0x43e2c0).
    int LoadBgmFormat(char *path);
    // Starts streaming the wave music from the file at `path`
    // (thbgm.dat), as bgmFormat's first track, after stopping what
    // plays, and starts the thread that streams it. Returns 0 or -1
    // (0x43efa0).
    int LoadBgm(char *path);
    // Switches the wave music to the track named as the file name of
    // `path`. Returns 0, or -1 when no music is loaded (0x43e9e0).
    int SwitchBgmTrack(char *path);
    // Reads the track named as the file name of `path` out of bgmPath
    // into preload slot `slot`, unless it is there already. Does nothing
    // unless the config's preloadBgm is set. Returns 0 or -1 (0x43ea30).
    int PreloadBgm(int slot, char *path);
    // Starts the wave music from preload slot `slot`, as LoadBgm does,
    // or switches the stream to its track without preloading. Returns 0
    // or -1 (0x43f0a0).
    int PlayPreloadedBgm(int slot);
    // Loads the sound effects ("data/wav/se_plst00.wav", ...) (0x43ec80).
    int LoadSounds();
    // Carries out the commands' next stages and plays the queued sound
    // effects. Returns the first command's type (0x43f1f0).
    int PlayQueuedSounds();
    // Fades the wave music out over `seconds`. An out-of-line copy
    // (0x43e4e0).
    void FadeOutMusic(float seconds)
    {
        if (this->bgm != NULL)
        {
            this->bgm->FadeOut(seconds);
        }
    }
    // Frees preload slot `slot`'s data, if any. An out-of-line copy
    // (0x43e4a0).
    void FreePreloadedBgm(int slot)
    {
        if (this->bgmPreloadData[slot] != NULL)
        {
            g_MemoryManager.Free(this->bgmPreloadData[slot]);
            this->bgmPreloadData[slot] = NULL;
        }
    }
    // Queues sound effect `idx` with `arg` to start on the next update:
    // a sound already queued collects up to 0x80 arguments, and up to 12
    // sounds are queued; th08's QueueSound (0x43e2f0).
    void QueueSound(int idx, int arg);
    // QueueSound with the pan for horizontal position `x`, as th08's
    // QueueSoundPanned (0x43e380).
    void QueueSoundPanned(int idx, float x);
    // Queues a command of `type` with `arg` and `path` in the first free
    // entry of `commands` (0x43ee40).
    void QueueSoundCommand(int type, int arg, char *path);
    // Queues the "UnPause" command (7). Inline in th08; this is the copy
    // at 0x423498, in an /O1 object.
    void UnPause();
    // Steps the wave music's fades, as th08's (0x42fbe0).
    void UpdateBgmFades();

    IDirectSound8 *dsound;
    DWORD unusedDword004;
    // The loaded sound files' buffers.
    IDirectSoundBuffer *soundBuffers[0x80];
    // Each sound effect's own buffer.
    IDirectSoundBuffer *sounds[0x80];
    // Set from g_SoundInfos[].unk_6 when a sound is queued (nothing reads
    // either); -1 at first.
    int unk_408[0x80];
    // A silent buffer, looping from start-up.
    IDirectSoundBuffer *silence;
    // The window the sound timer posts to.
    HWND hwnd;
    // NULL until Initialize succeeds.
    CSoundManager *manager;
    // The thread streaming the wave music and its ID.
    DWORD notifyThreadId;
    HANDLE notifyThread;
    DWORD unusedDword61c;
    // The sound effects to play, or -1.
    int queue[0xc];
    // How many arguments each queued sound has, and the arguments.
    int queueArgCounts[0xc];
    int queueArgs[0xc][0x80];
    // The thbgm.fmt entries of the preloaded tracks, their data, and
    // its size.
    BgmFormat *bgmEntries[0x10];
    void *bgmPreloadData[0x10];
    void *bgmPreloadCursor[0x10];
    DWORD bgmPreloadSize[0x10];
    // The preloaded track playing.
    int bgmPlaying;
    // The contents of thbgm.fmt.
    BgmFormat *bgmFormat;
    // The commands for the wave music, in order.
    SoundCommand commands[0x20];
    // The file names of the preloaded tracks (th08 0x10 of them).
    char bgmNames[0x20][0x100];
    // The file the wave music streams from.
    char bgmPath[0x100];
    // The wave music playing; NULL when stopped.
    CStreamingSound *bgm;
    // Signalled as the wave music plays.
    HANDLE notifyEvent;
    DWORD unusedDword6210;
    // Added to every BgmFormat offset by CWaveFile::ResetFile.
    DWORD bgmFileOffset;
    // The music and sound effect volumes in percent, 100 at first.
    int bgmVolume;
    int seVolume;
    DWORD bgmBufferVolume;
};
ASSERT_SIZE(SoundPlayer, 0x6224);
extern SoundPlayer g_SoundPlayer;
