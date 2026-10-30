#include "network.h"
#include "unsorted.h"

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

HRESULT WINAPI NetworkMessageHandler(PVOID context, DWORD type, PVOID message)
{
    return g_Network->HandleMessage(context, type, message);
}
