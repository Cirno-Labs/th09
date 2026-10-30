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
