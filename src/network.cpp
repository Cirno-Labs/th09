#include "network.h"
#include "unsorted.h"

// The SDK's header declares it without C linkage.
extern "C"
{
#include <dxerr8.h>
}

// The application GUID of the game's DirectPlay sessions (0x490c3c).
static const GUID g_SessionGuid = {0x9fd5ec50, 0x740a, 0x4fdb, {0x90, 0x9d, 0x2f, 0x24, 0xe1, 0x52, 0x82, 0xea}};

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
