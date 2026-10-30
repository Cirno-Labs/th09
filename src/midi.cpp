#include "midi.h"
#include "unsorted.h"

MidiDevice::MidiDevice()
{
    this->handle = 0;
    this->deviceId = 0;
}

int MidiDevice::Close()
{
    if (this->handle == 0)
    {
        return -1;
    }
    midiOutReset(this->handle);
    midiOutClose(this->handle);
    this->handle = 0;
    return 0;
}

int MidiDevice::SendLongMsg(LPMIDIHDR header)
{
    if (this->handle == 0)
    {
        return 0;
    }
    if (midiOutPrepareHeader(this->handle, header, sizeof(MIDIHDR)) != MMSYSERR_NOERROR)
    {
        return 1;
    }
    return midiOutLongMsg(this->handle, header, sizeof(MIDIHDR)) != MMSYSERR_NOERROR;
}

int MidiDevice::SendShortMsg(BYTE status, BYTE data1, BYTE data2)
{
    MidiShortMsg msg;

    if (this->handle == 0)
    {
        return 0;
    }
    msg.bytes[0] = status;
    msg.bytes[1] = data1;
    msg.bytes[2] = data2;
    return midiOutShortMsg(this->handle, msg.dword) != MMSYSERR_NOERROR;
}

MmTimer::MmTimer()
{
    timeGetDevCaps(&this->caps, sizeof(this->caps));
    this->timerId = 0;
}

// Empty; the linker folded it with DebugPrint (0x43d2b0).
void MmTimer::OnTimer()
{
}

int MmTimer::Stop()
{
    if (this->timerId != 0)
    {
        timeKillEvent(this->timerId);
    }
    timeEndPeriod(this->caps.wPeriodMin);
    this->timerId = 0;
    return 1;
}

void CALLBACK MmTimer::DefaultCallback(UINT id, UINT msg, DWORD_PTR user, DWORD_PTR dw1, DWORD_PTR dw2)
{
    MmTimer *timer = (MmTimer *)user;
    timer->OnTimer();
}

WORD SwapBytes16(WORD value)
{
    union
    {
        WORD word;
        BYTE bytes[2];
    } swapped;

    swapped.bytes[0] = ((BYTE *)&value)[1];
    swapped.bytes[1] = ((BYTE *)&value)[0];
    return swapped.word;
}

DWORD SwapBytes32(DWORD value)
{
    union
    {
        DWORD dword;
        BYTE bytes[4];
    } swapped;

    swapped.bytes[0] = ((BYTE *)&value)[3];
    swapped.bytes[1] = ((BYTE *)&value)[2];
    swapped.bytes[2] = ((BYTE *)&value)[1];
    swapped.bytes[3] = ((BYTE *)&value)[0];
    return swapped.dword;
}

DWORD ReadVarLen(BYTE **cursor)
{
    DWORD value = 0;
    BYTE byte;

    do
    {
        byte = **cursor;
        (*cursor)++;
        value = (value << 7) + (byte & 0x7f);
    } while (byte & 0x80);
    return value;
}

void MidiPlayer::Rewind()
{
    int i;
    MidiTrack *track;

    track = this->tracks;
    this->volumeScale = 1.0f;
    this->framesSinceRewind = 0;
    this->fading = 0;
    this->ticks = 0;
    this->baseTime = 0;
    for (i = 0; i < this->trackCount; i++, track++)
    {
        track->cursor = track->data;
        track->loopCursor = track->cursor;
        track->active = 1;
        track->delta = ReadVarLen(&track->cursor);
    }
}

int MidiPlayer::FadeOut(int duration)
{
    this->volumeScale = 0.0f;
    this->fadeDuration = duration;
    this->fadeElapsed = 0;
    this->framesSinceRewind = 0;
    this->fading = 1;
    return 0;
}

void MidiPlayer::SendVolume(int offset)
{
    int cc;
    int i;
    int chStatus;
    int data2;
    int volume;

    if (this->suppressVolume != 0)
    {
        return;
    }
    cc = 7;
    for (i = 0; i < 0x10; i++)
    {
        chStatus = (BYTE)(i + 0xb0);
        volume = (int)(this->channels[i].volume * this->volumeScale) + offset;
        if (volume < 0)
        {
            volume = 0;
        }
        else if (volume > 0x7f)
        {
            volume = 0x7f;
        }
        data2 = (BYTE)volume;
        this->device.SendShortMsg(chStatus, cc, data2);
    }
}

void MidiTimer::OnTimer()
{
    QueryPerformanceCounter(&g_MidiTimerLastCounter);
}

void MidiTimer::StopTimer()
{
    this->Stop();
}

MidiDevice::~MidiDevice()
{
    this->Close();
}

int MidiDevice::Open(UINT deviceId)
{
    if (this->handle != 0)
    {
        if (this->deviceId != deviceId)
        {
            this->Close();
        }
        else
        {
            return 0;
        }
    }
    this->deviceId = deviceId;
    return midiOutOpen(&this->handle, deviceId, (DWORD_PTR)g_App.hwnd, 0, CALLBACK_WINDOW) != MMSYSERR_NOERROR;
}
