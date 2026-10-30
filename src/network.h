// The network play object.

#pragma once

#include <windows.h>
#include <dplay8.h>

#include "common.h"

// The network play's state ("NetworkInf", 0xd4 bytes), at 0x4b42d0.
// Only what is known so far.
struct Network
{
    // Reads IP.txt: the host name, the port, the sync mode and the side
    // (0x4324c0).
    void Init();
    // Sets and clears unknownA8 (0x42e9a0, 0x42e9b0).
    void SetUnknownA8();
    void ClearUnknownA8();
    // Closes the connection and releases its interfaces (0x432f70).
    void Close();
    // The entry of unknown1c for the side unknownAc is not (0x42e980).
    int GetOtherUnknown1c();
    // Closes the peer and releases the addresses and the peer
    // (0x432340).
    void ReleaseInterfaces();
    // Sends `size` bytes of `data` to the other side's player, if
    // connected; returns 0 if not (0x4323b0).
    HRESULT Send(void *data, DWORD size);
    // Whether the peer has TCP/IP among its service providers; the
    // argument is unused (0x432770).
    BOOL IsServiceProviderValid(const GUID *serviceProvider);
    // Clears unknown1c and creates the TCP/IP address on `port` to host
    // on (0x432840).
    HRESULT CreateDeviceAddress();
    // Creates the TCP/IP address of the host to connect to, hostName on
    // `port` (0x4328f0).
    HRESULT CreateHostAddress();
    // Hosts the session on deviceAddress (0x432a10).
    HRESULT Host();
    // Starts connecting to the session at hostAddress through
    // deviceAddress, into connectHandle (0x432ac0).
    HRESULT Connect();
    // Handles a DirectPlay message (0x432b70).
    HRESULT HandleMessage(PVOID context, DWORD type, PVOID message);

    IDirectPlay8Peer *peer;
    IDirectPlay8Address *deviceAddress;
    unsigned char pad_8[4];
    IDirectPlay8Address *unknownC;
    IDirectPlay8Address *hostAddress;
    DPNHANDLE sendHandle;
    DPNHANDLE connectHandle;
    // One per side.
    int unknown1c[2];
    // The host to connect to, from IP.txt's IPAddress.
    char hostName[0x80];
    // From IP.txt's Port.
    DWORD port;
    // While set, the menus do not skip the entries that are not open.
    int unknownA8;
    // The local side, 0 or 1, so it seems.
    int unknownAc;
    unsigned char pad_b0[0xb8 - 0xb0];
    // Cleared by App::ResetRun.
    int unknownB8;
    // Set by IP.txt's Sync LAN.
    int isLan;
    // From IP.txt's Side: 0 for 1P, 1 for 2P, 2 for Auto.
    int side;
    // From IP.txt's Sync: 60 for LAN and High, 30 for Low, 20 for Bad.
    unsigned char syncFrames;
    unsigned char pad_c5[0xd4 - 0xc5];
};
ASSERT_SIZE(Network, 0xd4);
extern Network *g_Network;
// Logs the system's message for `hr`, else DirectX's name of it
// (0x432410).
void LogNetworkError(HRESULT hr);
// The DirectPlay message handler: hands the message to g_Network
// (0x432ea0).
HRESULT WINAPI NetworkMessageHandler(PVOID context, DWORD type, PVOID message);
