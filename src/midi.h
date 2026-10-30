// MIDI playback through WinMM, as th08's.

#pragma once

#include <windows.h>
#include <mmsystem.h>

#include "common.h"

// A short MIDI message as midiOutShortMsg takes it: status and two data
// bytes, low byte first.
union MidiShortMsg
{
    DWORD dword;
    BYTE bytes[4];
};

// Swaps the bytes of a big-endian 16-bit value from a MIDI file
// (0x43d100).
WORD SwapBytes16(WORD value);

// Swaps the bytes of a big-endian 32-bit value from a MIDI file. Inline
// in th07 (0x43d120).
DWORD SwapBytes32(DWORD value);

// The header of a chunk in a MIDI file ("MThd", "MTrk").
struct MidiChunk
{
    char id[4];
    // Big-endian.
    DWORD length;
};

// Reads a MIDI variable-length quantity (7 bits per byte, high bit set on
// all but the last) and advances the cursor past it (0x43d150).
DWORD ReadVarLen(BYTE **cursor);

// Wraps a MIDI output device (midiOutOpen and friends).
class MidiDevice
{
public:
    // 0x43cfd0, and 0x43d2c0.
    MidiDevice();
    ~MidiDevice();

    // Returns -1 if no device is open, 0 once it is closed (0x43cfe0).
    int Close();

    // Opens the given device, closing another open one first. Returns
    // nonzero on failure, 0 on success or if the device is already open
    // (0x43d2d0).
    int Open(UINT deviceId);

    // Prepares and sends a system-exclusive message. Returns 0 if no
    // device is open, nonzero if either step fails (0x43d010).
    int SendLongMsg(LPMIDIHDR header);

    // Sends a channel message. Returns 0 if no device is open, nonzero on
    // failure (0x43d060).
    int SendShortMsg(BYTE status, BYTE data1, BYTE data2);

    HMIDIOUT handle;
    UINT deviceId;
};

// A periodic WinMM multimedia timer. By default the timer calls OnTimer.
class MmTimer
{
public:
    // 0x43d0a0, 0x43d320.
    MmTimer();
    ~MmTimer();

    // Starts a periodic timer event every `delay` ms at the finest
    // resolution, calling `callback` with `user`, or OnTimer if there is no
    // callback. Returns the timer ID, 0 on failure (0x43d340).
    UINT Start(UINT delay, LPTIMECALLBACK callback, DWORD_PTR user);

    // Kills the timer event, if any, and ends the timer period. Returns 1
    // (0x43d0c0).
    int Stop();

    // Called on every tick when the timer was started without a callback
    // of its own. Empty here; the linker folded it with DebugPrint
    // (0x43d2b0).
    virtual void OnTimer();

    // The timeSetEvent callback used when Start gets none: calls OnTimer
    // on the timer passed as user data (0x43d0f0).
    static void CALLBACK DefaultCallback(UINT id, UINT msg, DWORD_PTR user, DWORD_PTR dw1, DWORD_PTR dw2);

    UINT timerId;
    TIMECAPS caps;
};
ASSERT_SIZE(MmTimer, 0x10);

// One track of a parsed MIDI file. Size from the stride of
// MidiPlayer::tracks.
struct MidiTrack
{
    // Set when the track is rewound.
    int active;
    // Time of the next event, from the delta times read by ReadVarLen.
    int delta;
    // The length of `data`.
    DWORD length;
    // The last status byte, for running status.
    BYTE runningStatus;
    // The track's event data.
    BYTE *data;
    // Read position in `data`.
    BYTE *cursor;
    // `cursor` and `delta` at the loop start, saved by controller 2 and
    // restored by controller 4. The start of the data after a rewind.
    BYTE *loopCursor;
    int loopDelta;
};
ASSERT_SIZE(MidiTrack, 0x20);

// The state of one of the 16 MIDI channels. Size from the stride of
// MidiPlayer::channels.
struct MidiChannel
{
    // One bit per note that is on, note n at bit n % 8 of byte n / 8.
    BYTE notes[0x10];
    BYTE program;
    // Controllers 0, 10, 91 and 93.
    BYTE bank;
    BYTE pan;
    BYTE reverb;
    BYTE chorus;
    // Channel volume (controller 7) before scaling.
    BYTE volume;
    // `volume` scaled by volumeScale, as last sent.
    BYTE scaledVolume;
};
ASSERT_SIZE(MidiChannel, 0x17);

// Plays MIDI files on a MidiDevice, driven by its own timer, as th08's
// MidiPlayer, with its layout.
class MidiPlayer : public MmTimer
{
public:
    MidiPlayer();
    // 0x43e090.
    ~MidiPlayer();

    virtual void OnTimer();

    // Frees the MIDI file loaded into a slot (0x43d550).
    void UnloadFile(int slot);
    // Loads a MIDI file into a slot, stopping playback first if that slot
    // is playing. Returns -1 on failure, otherwise 0 (0x43e0d0).
    int LoadFile(int slot, char *path);
    // Sends every channel's volume, scaled and offset by `offset`, unless
    // suppressVolume is set (0x43d210).
    void SendVolume(int offset);
    // Stops playback and closes the device. Returns -1 if nothing is
    // loaded, otherwise 0 (0x43de30).
    int Stop();
    // Unprepares and frees a header from `headers` (0x43d790).
    int ReleaseHeader(LPMIDIHDR header);
    // Plays the loaded tracks on the MIDI mapper, ticking every 1 ms.
    // Returns -1 if nothing is loaded, otherwise 0 (0x43d750).
    int Play();
    // Stops, parses slot `slot` and plays it. Inline in th08; this is
    // the copy at 0x423361, in an /O1 object.
    void Restart(int slot);
    // Rewinds the tracks (0x43d170).
    void Rewind();
    // Starts fading the music out over `duration` ms. Returns 0
    // (0x43d1e0).
    int FadeOut(int duration);
    // Frees the parsed tracks (0x43d580).
    void FreeTracks();
    // Parses the file in a slot into tracks. Returns -1 if the slot is
    // empty, otherwise 0.
    int ParseFile(int slot);
    // Loads and parses a MIDI file through the scratch slot 31. Returns -1
    // if it cannot be loaded, otherwise 0 (0x43e130).
    int Load(char *path);
    // Handles the next event of a track and reads the time of the one
    // after.
    void ProcessEvent(MidiTrack *track);
    // The slot of the file being played, -1 for none. LoadFile stops
    // playback before it replaces that file.
    int playingSlot;
    // Long messages being sent; released by ReleaseHeader.
    LPMIDIHDR headers[0x20];
    // The entry of `headers` the next long message goes in.
    int nextHeader;
    // MIDI files loaded by LoadFile, freed by UnloadFile.
    void *fileData[0x20];
    // The number of entries in `tracks`.
    int trackCount;
    // From the file header: the format and the time division.
    int format;
    int division;
    // Microseconds per quarter note, from the tempo meta event.
    int tempo;
    // Timer ticks since the start.
    unsigned __int64 ticks;
    // The time, in ms, played at earlier tempos; added to the time computed
    // from `ticks`, which restart at each tempo change.
    unsigned __int64 baseTime;
    // Parsed tracks, `trackCount` of them.
    MidiTrack *tracks;
    MidiDevice device;
    unsigned char pad_144[0x10];
    MidiChannel channels[0x10];
    // Added to every note number.
    char transpose;
    // Scales every channel's volume.
    float volumeScale;
    // The volume last sent while fading, volumeScale * 128.
    int lastVolume;
    int unusedAfterConstruct2d0;
    int suppressVolume;
    int unusedAfterConstruct2d8;
    int framesSinceRewind;
    // Set while fading out.
    int fading;
    // In ms, set by FadeOut.
    int fadeDuration;
    // Ticks of the fade so far.
    int fadeElapsed;
    // tempo, ticks and baseTime at the loop start (controller 2).
    int loopTempo;
    unsigned __int64 loopTicks;
    unsigned __int64 loopBaseTime;
};
ASSERT_SIZE(MidiPlayer, 0x300);

// The performance counter at the last tick of MidiTimer's timer.
extern LARGE_INTEGER g_MidiTimerLastCounter;


// A 16 ms MmTimer that keeps the performance counter of its last tick in
// g_MidiTimerLastCounter. Not in th06.
class MidiTimer : public MmTimer
{
public:
    // Stores the performance counter (0x43d290).
    virtual void OnTimer();

    // Starts the timer, ticking every 16 ms (0x43de20).
    void StartTimer();
    // Stops the timer (0x43d2a0).
    void StopTimer();
};
