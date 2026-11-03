// The network play object.

#pragma once

#include <windows.h>
#include <dplay8.h>

#include "common.h"

// What the two sides send each other (0xc bytes).
struct NetPacket
{
    // 0 for a frame's input, 1 for a check, 2 for the session setup.
    BYTE type;
    // The sending side.
    char side;
    WORD seed;
    // The setup sends the sync frames in it.
    int value;
    // The input, or the setup's isLan.
    union
    {
        int unknown8;
        struct
        {
            WORD input;
            WORD checksum;
        };
    };
};
ASSERT_SIZE(NetPacket, 0xc);

// The network play's state ("NetworkInf", 0xd4 bytes), at 0x4b42d0.
// Only what is known so far.
struct Network
{
    // Zeroes the object and sets syncFrames to 60 (0x42cc10).
    Network();
    // Reads IP.txt: the host name, the port, the sync mode and the side
    // (0x4324c0).
    void Init();
    // Sets and clears unknownA8 (0x42e9a0, 0x42e9b0).
    void SetUnknownA8();
    void ClearUnknownA8();
    // Cancels and closes the connection, releases the addresses and
    // initializes the peer anew; always returns 1 (0x432f70).
    int Close();
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
    // Creates and initializes the peer and checks for TCP/IP
    // (0x432eb0).
    HRESULT CreatePeer();

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
    // Set when a send completes, or a check packet matches.
    int unknownB0;
    // Set after the initial frame records and setup packet are sent.
    int initialFramesQueued;
    // Cleared by App::ResetRun.
    int unknownB8;
    // Set by IP.txt's Sync LAN.
    int isLan;
    // From IP.txt's Side: 0 for 1P, 1 for 2P, 2 for Auto.
    int side;
    // From IP.txt's Sync: 60 for LAN and High, 30 for Low, 20 for Bad.
    unsigned char syncFrames;
    unsigned char pad_c5[3];
    // The last input packet received from the other side.
    NetPacket lastPacket;
};
ASSERT_SIZE(Network, 0xd4);
extern Network *g_Network;
// Logs the system's message for `hr`, else DirectX's name of it
// (0x432410).
void LogNetworkError(HRESULT hr);
// The DirectPlay message handler: hands the message to g_Network
// (0x432ea0).
HRESULT WINAPI NetworkMessageHandler(PVOID context, DWORD type, PVOID message);
