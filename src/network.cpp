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

HRESULT WINAPI NetworkMessageHandler(PVOID context, DWORD type, PVOID message)
{
    return g_Network->HandleMessage(context, type, message);
}
