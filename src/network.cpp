// cflags: /Ob1
#include "network.h"
#include "unsorted.h"

extern "C" {
#include <dxerr8.h>
}

// The application GUID of the game's DirectPlay sessions (0x490c3c).
static const GUID g_SessionGuid = {0x9fd5ec50, 0x740a, 0x4fdb, {0x90, 0x9d, 0x2f, 0x24, 0xe1, 0x52, 0x82, 0xea}};

// Reconstruction hints: see docs/references.md.
// This helper is inline so its early exits stay inside the parser.
static inline bool SkipConfigValueSpaces(char *&cursor, int *remaining, char *value)
{
    while (*cursor == ' ' || *cursor == '\t')
    {
        if (*remaining <= 0)
        {
            return false;
        }
        if (*cursor == '\n')
        {
            value[0] = '\0';
            return false;
        }
        cursor++;
        (*remaining)--;
    }
    return true;
}

// Reads the value of `key` from the IP.txt line at `cursor` into `value`
// (0x100 bytes): "key = value", where the value ends at a blank, a line
// end or a ';' comment. Counts the bytes read off `remaining`. Returns
// where it stopped, or NULL if the line is not `key`'s (0x432240).
static char *ReadConfigValue(char *cursor, int *remaining, char *key, char *value)
{
    int i;

    memset(value, 0, 0x100);
    if (strncmp(cursor, key, strlen(key)) == 0)
    {
        cursor += strlen(key);
        while (*cursor == ' ' || *cursor == '\t')
        {
            if (*remaining <= 0 || *cursor == '\n')
            {
                return cursor;
            }
            cursor++;
            (*remaining)--;
        }
        if (*remaining > 0 && *cursor == '=')
        {
            cursor++;
            (*remaining)--;
            if (SkipConfigValueSpaces(cursor, remaining, value))
            {
                if (*remaining > 0 && *cursor != ';')
                {
                    for (i = 0; *cursor != ' ' && *cursor != '\t' && *cursor != '\n' && *cursor != '\r' &&
                                *cursor != ';' && *remaining > 0 && i < 0xff; i++)
                    {
                        value[i] = *cursor;
                        cursor++;
                        (*remaining)--;
                    }
                }
            }
        }
        return cursor;
    }
    return NULL;
}

// Only the parser above expands an inline: the rest of the file calls
// global.h's, such as MemoryManager::Alloc, out of line.
#pragma inline_depth(0)

void Network::ReleaseInterfaces()
{
    if (this->peer != NULL)
    {
        this->peer->Close(0);
    }
    if (this->deviceAddress != NULL)
    {
        this->deviceAddress->Release();
        this->deviceAddress = NULL;
    }
    if (this->unknownC != NULL)
    {
        this->unknownC->Release();
        this->unknownC = NULL;
    }
    if (this->hostAddress != NULL)
    {
        this->hostAddress->Release();
        this->hostAddress = NULL;
    }
    if (this->peer != NULL)
    {
        this->peer->Release();
        this->peer = NULL;
    }
}

HRESULT Network::Send(void *data, DWORD size)
{
    DPN_BUFFER_DESC desc;

    if (this->unknownA8 == 0)
    {
        return 0;
    }
    if (this->unknown1c[1 - this->unknownAc] == 0)
    {
        return 0;
    }
    desc.dwBufferSize = size;
    desc.pBufferData = (BYTE *)data;
    return this->peer->SendTo(this->unknown1c[1 - this->unknownAc], &desc, 1, 0, NULL, &this->sendHandle,
                              DPNSEND_PRIORITY_HIGH | DPNSEND_NONSEQUENTIAL | DPNSEND_GUARANTEED |
                                  DPNSEND_COMPLETEONPROCESS);
}

void LogNetworkError(HRESULT hr)
{
    char text[0x800];
    LPSTR message;

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                   hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message, 0, NULL);
    if (message != NULL)
    {
        LogError(&g_Log, message);
        LocalFree(message);
        return;
    }
    strcpy(text, DXGetErrorString8A(hr));
    strcat(text, "\r\n");
    LogError(&g_Log, text);
}

void Network::Init()
{
    char value[0x100];
    char *next;
    char *data;
    int size;
    char *cursor;

    this->unknownA8 = 0;
    if (!FileExists("IP.txt"))
    {
        return;
    }
    data = (char *)LoadFileData("IP.txt", &size, 1);
    memset(this->hostName, 0, sizeof(this->hostName));
    this->isLan = 0;
    this->syncFrames = 30;
    cursor = data;
    while (size > 0)
    {
        char c = *cursor;
        if (c == ' ' || c == '\n' || c == '\t' || c == '\r')
        {
            goto skipCharacter;
        }
        if (c == ';')
        {
            do
            {
                if (size <= 0)
                {
                    goto done;
                }
                cursor++;
                size--;
            } while (*cursor != '\n');
            continue;
        }
        if ((next = ReadConfigValue(cursor, &size, "IPAddress", value)) != NULL)
        {
            cursor = next;
            strcpy(this->hostName, value);
        }
        else if ((next = ReadConfigValue(cursor, &size, "Port", value)) != NULL)
        {
            cursor = next;
            this->port = atoi(value);
        }
        else if ((next = ReadConfigValue(cursor, &size, "Sync", value)) != NULL)
        {
            cursor = next;
            if (strcmp(value, "LAN") == 0)
            {
                this->isLan = 1;
                this->syncFrames = 60;
            }
            else if (strcmp(value, "High") == 0)
            {
                this->isLan = 0;
                this->syncFrames = 60;
            }
            else if (strcmp(value, "Low") == 0)
            {
                this->isLan = 0;
                this->syncFrames = 30;
            }
            else if (strcmp(value, "Bad") == 0)
            {
                this->isLan = 0;
                this->syncFrames = 20;
            }
        }
        else if ((next = ReadConfigValue(cursor, &size, "Side", value)) != NULL)
        {
            cursor = next;
            if (strcmp(value, "Auto") == 0)
            {
                this->side = 2;
            }
            else if (strcmp(value, "1P") == 0)
            {
                this->side = 0;
            }
            else if (strcmp(value, "2P") == 0)
            {
                this->side = 1;
            }
        }
        else
        {
        skipCharacter:
            cursor++;
            size--;
        }
    }
done:
    g_MemoryManager.Free(data);
}

BOOL Network::IsServiceProviderValid(const GUID *serviceProvider)
{
    HRESULT hr;
    DPN_SERVICE_PROVIDER_INFO *info = NULL;
    DWORD count = 0;
    DWORD size = 0;

    hr = this->peer->EnumServiceProviders(&CLSID_DP8SP_TCPIP, NULL, NULL, &size, &count, 0);
    if (hr != DPNERR_BUFFERTOOSMALL)
    {
        printf("Failed Enumerating Service Providers:  0x%x\r\n", hr);
        goto cleanup;
    }
    info = (DPN_SERVICE_PROVIDER_INFO *)g_MemoryManager.Alloc(size, GLOBAL_H);
    if (FAILED(hr = this->peer->EnumServiceProviders(&CLSID_DP8SP_TCPIP, NULL, info, &size, &count, 0)))
    {
        LogError(&g_Log, "サービスプロバイダ列挙失敗:\t0x%x\r\n", hr);
        goto cleanup;
    }
    if (count == 0)
    {
        hr = E_FAIL;
    }
cleanup:
    if (info != NULL)
    {
        g_MemoryManager.Free(info);
    }
    return SUCCEEDED(hr);
}

HRESULT Network::CreateDeviceAddress()
{
    HRESULT hr;

    this->unknown1c[0] = 0;
    this->unknown1c[1] = 0;
    hr = CoCreateInstance(CLSID_DirectPlay8Address, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay8Address,
                          (LPVOID *)&this->deviceAddress);
    if (FAILED(hr))
    {
        LogError(&g_Log, "Device Address作成失敗\r\n");
        return hr;
    }
    hr = this->deviceAddress->SetSP(&CLSID_DP8SP_TCPIP);
    if (FAILED(hr))
    {
        LogError(&g_Log, "サービスプロバイダ設定失敗:\t0x%X\r\n", hr);
        return hr;
    }
    hr = this->deviceAddress->AddComponent(DPNA_KEY_PORT, &this->port, sizeof(this->port), DPNA_DATATYPE_DWORD);
    if (FAILED(hr))
    {
        LogError(&g_Log, "ポート設定失敗:\t0x%X\r\n", hr);
    }
    return hr;
}

HRESULT Network::CreateHostAddress()
{
    WCHAR hostName[0x100];
    HRESULT hr;

    mbstowcs(hostName, this->hostName, 0x100);
    hr = CoCreateInstance(CLSID_DirectPlay8Address, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay8Address,
                          (LPVOID *)&this->hostAddress);
    if (FAILED(hr))
    {
        LogError(&g_Log, "Host Address作成失敗\r\n");
        return hr;
    }
    hr = this->hostAddress->SetSP(&CLSID_DP8SP_TCPIP);
    if (FAILED(hr))
    {
        LogError(&g_Log, "サービスプロバイダ設定失敗:\t0x%X\r\n", hr);
        return hr;
    }
    hr = this->hostAddress->AddComponent(DPNA_KEY_HOSTNAME, hostName, (wcslen(hostName) + 1) * sizeof(WCHAR),
                                         DPNA_DATATYPE_STRING);
    if (FAILED(hr))
    {
        LogError(&g_Log, "ホストアドレスIP設定失敗\r\n");
        return hr;
    }
    hr = this->hostAddress->AddComponent(DPNA_KEY_PORT, &this->port, sizeof(this->port), DPNA_DATATYPE_DWORD);
    if (FAILED(hr))
    {
        LogError(&g_Log, "ホストアドレスポート設定失敗\r\n");
    }
    return hr;
}

HRESULT Network::Host()
{
    WCHAR sessionName[0x80];
    DPN_APPLICATION_DESC desc;
    HRESULT hr;

    mbstowcs(sessionName, "TH09YoukaiMatch", 0x80);
    ZeroMemory(&desc, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.guidApplication = g_SessionGuid;
    desc.pwszSessionName = sessionName;
    hr = this->peer->Host(&desc, &this->deviceAddress, 1, NULL, NULL, NULL, DPNHOST_OKTOQUERYFORADDRESSING);
    if (FAILED(hr))
    {
        LogError(&g_Log, "新しいセッション作成失敗:\t 0x%X\r\n", hr);
        LogNetworkError(hr);
    }
    return hr;
}

HRESULT Network::Connect()
{
    WCHAR sessionName[0x80];
    DPN_APPLICATION_DESC desc;
    HRESULT hr;

    mbstowcs(sessionName, "TH09YoukaiMatch", 0x80);
    ZeroMemory(&desc, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.guidApplication = g_SessionGuid;
    desc.pwszSessionName = sessionName;
    hr = this->peer->Connect(&desc, this->hostAddress, this->deviceAddress, NULL, NULL, NULL, 0, NULL, NULL,
                             &this->connectHandle, DPNCONNECT_OKTOQUERYFORADDRESSING);
    if (hr != DPNSUCCESS_PENDING && hr != DPN_OK)
    {
        LogNetworkError(hr);
    }
    return hr;
}

HRESULT Network::HandleMessage(PVOID context, DWORD type, PVOID message)
{
    NetPacket *packet;
    DPN_PLAYER_INFO *info;
    DWORD size;
    HRESULT hr;
    int i;

    switch (type)
    {
    case DPN_MSGID_RECEIVE:
        g_App.EnterCriticalSection(4);
        packet = (NetPacket *)((DPNMSG_RECEIVE *)message)->pReceiveData;
        if (packet->type == 0)
        {
            if (packet->side != g_Network->unknownAc)
            {
                this->lastPacket = *packet;
                g_App.AddRecord(packet->side, packet->value, packet->unknown8, packet->seed);
            }
        }
        else if (packet->type == 1)
        {
            if (g_App.unknown458 == packet->value && packet->value == packet->seed)
            {
                this->unknownB0 = 1;
            }
        }
        else if (packet->type == 2)
        {
            if (packet->side == this->unknownAc)
            {
                this->unknownAc = 1 - packet->side;
            }
            if (this->unknownAc == 1)
            {
                g_Rng.SetSeed(packet->seed);
                this->syncFrames = packet->value;
                this->isLan = packet->unknown8;
                if (60 / this->syncFrames > 1)
                {
                    for (i = 0; i < 60 / this->syncFrames - 1; i++)
                    {
                        g_App.AddRecord(0, i - 9998, 0, g_Rng.GetSeed());
                        g_App.AddRecord(1, i - 9998, 0, g_Rng.GetSeed());
                    }
                }
            }
        }
        g_App.unknown464 = g_App.unknown46c;
        g_App.LeaveCriticalSection(4);
        break;
    case DPN_MSGID_CREATE_PLAYER:
        g_App.EnterCriticalSection(4);
        size = 0;
        this->peer->GetPeerInfo(((DPNMSG_CREATE_PLAYER *)message)->dpnidPlayer, NULL, &size, 0);
        info = (DPN_PLAYER_INFO *)new BYTE[size];
        memset(info, 0, size);
        info->dwSize = sizeof(DPN_PLAYER_INFO);
        hr = this->peer->GetPeerInfo(((DPNMSG_CREATE_PLAYER *)message)->dpnidPlayer, info, &size, 0);
        if (hr != DPN_OK)
        {
            delete info;
            g_App.LeaveCriticalSection(4);
            return hr;
        }
        if (info->dwPlayerFlags & DPNPLAYER_LOCAL)
        {
            this->unknown1c[this->unknownAc] = ((DPNMSG_CREATE_PLAYER *)message)->dpnidPlayer;
        }
        else
        {
            this->unknown1c[1 - this->unknownAc] = ((DPNMSG_CREATE_PLAYER *)message)->dpnidPlayer;
        }
        delete info;
        g_App.LeaveCriticalSection(4);
        break;
    case DPN_MSGID_DESTROY_PLAYER:
        g_App.EnterCriticalSection(4);
        if (this->unknown1c[0] == ((DPNMSG_DESTROY_PLAYER *)message)->dpnidPlayer)
        {
            this->unknown1c[0] = 0;
        }
        if (this->unknown1c[1] == ((DPNMSG_DESTROY_PLAYER *)message)->dpnidPlayer)
        {
            this->unknown1c[1] = 0;
        }
        g_App.LeaveCriticalSection(4);
        break;
    case DPN_MSGID_SEND_COMPLETE:
        g_App.EnterCriticalSection(4);
        this->unknownB0 = 1;
        g_App.LeaveCriticalSection(4);
        break;
    case DPN_MSGID_CONNECT_COMPLETE:
        g_App.EnterCriticalSection(4);
        if (((DPNMSG_CONNECT_COMPLETE *)message)->hResultCode == DPN_OK && this->unknownB8 == 1)
        {
            this->unknownB8 = 3;
            this->unknownAc = 1;
        }
        g_App.LeaveCriticalSection(4);
        break;
    }
    return 0;
}

HRESULT WINAPI NetworkMessageHandler(PVOID context, DWORD type, PVOID message)
{
    return g_Network->HandleMessage(context, type, message);
}

HRESULT Network::CreatePeer()
{
    HRESULT hr;

    hr = CoCreateInstance(CLSID_DirectPlay8Peer, NULL, CLSCTX_INPROC_SERVER, IID_IDirectPlay8Peer,
                          (LPVOID *)&this->peer);
    if (FAILED(hr))
    {
        LogError(&g_Log, "IDirectPlay8Peer 作成失敗:  0x%X\r\n", hr);
        LogNetworkError(hr);
        return hr;
    }
    hr = this->peer->Initialize(NULL, NetworkMessageHandler, 0);
    if (FAILED(hr))
    {
        LogError(&g_Log, "IDirectPlay8Peer 初期化失敗:  0x%X\r\n", hr);
        LogNetworkError(hr);
        return hr;
    }
    if (!IsServiceProviderValid(&CLSID_DP8SP_TCPIP))
    {
        hr = E_FAIL;
        LogError(&g_Log, "CLSID_DP8SP_TCPIP 使用不可\r\n");
        goto end;
    }
    LogPrintf(&g_Log, "DirectPlay が正常に初期化されたように見えた\r\n");
end:
    return hr;
}

int Network::Close()
{
    HRESULT hr;

    this->peer->CancelAsyncOperation(NULL, DPNCANCEL_ALL_OPERATIONS);
    this->peer->Close(0);
    if (this->deviceAddress != NULL)
    {
        this->deviceAddress->Release();
        this->deviceAddress = NULL;
    }
    if (this->unknownC != NULL)
    {
        this->unknownC->Release();
        this->unknownC = NULL;
    }
    if (this->hostAddress != NULL)
    {
        this->hostAddress->Release();
        this->hostAddress = NULL;
    }
    Sleep(100);
    hr = this->peer->Initialize(NULL, NetworkMessageHandler, 0);
    if (FAILED(hr))
    {
        LogError(&g_Log, "IDirectPlay8Peer 初期化失敗:  0x%X\r\n", hr);
        LogNetworkError(hr);
        return 1;
    }
    if (!IsServiceProviderValid(&CLSID_DP8SP_TCPIP))
    {
        LogError(&g_Log, "CLSID_DP8SP_TCPIP 使用不可\r\n");
        return 1;
    }
    this->unknown1c[0] = 0;
    this->unknown1c[1] = 0;
    return 1;
}
