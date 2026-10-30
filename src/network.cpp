#include "network.h"
#include "unsorted.h"

// The SDK's header declares it without C linkage.
extern "C"
{
#include <dxerr8.h>
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

HRESULT WINAPI NetworkMessageHandler(PVOID context, DWORD type, PVOID message)
{
    return g_Network->HandleMessage(context, type, message);
}
