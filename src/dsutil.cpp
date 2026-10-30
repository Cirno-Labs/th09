// The file from 0x43f7a0: the DirectX 8 SDK's dsutil.cpp, as the game
// builds it, as th08's, without th08's debug messages.

#include "dsutil.h"
#include "unsorted.h"

// From the SDK's dxutil.h.
#define SAFE_DELETE(p) { if(p) { delete (p); (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p); (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
// From the SDK's dxerr8.h, as in a release build.
#define DXTRACE_ERR(str, hr) (hr)

CSoundManager::CSoundManager()
{
    m_pDS = NULL;
}

CSoundManager::~CSoundManager()
{
    SAFE_RELEASE(m_pDS);
}

HRESULT CSoundManager::SetPrimaryBufferFormat(DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate)
{
    HRESULT hr;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    if (m_pDS == NULL)
        return CO_E_NOTINITIALIZED;

    // Get the primary buffer
    DSBUFFERDESC dsbd;
    ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
    dsbd.dwBufferBytes = 0;
    dsbd.lpwfxFormat = NULL;

    if (FAILED(hr = m_pDS->CreateSoundBuffer(&dsbd, &pDSBPrimary, NULL)))
        return DXTRACE_ERR(TEXT("CreateSoundBuffer"), hr);

    WAVEFORMATEX wfx;
    ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = (WORD)dwPrimaryChannels;
    wfx.nSamplesPerSec = dwPrimaryFreq;
    wfx.wBitsPerSample = (WORD)dwPrimaryBitRate;
    wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    if (FAILED(hr = pDSBPrimary->SetFormat(&wfx)))
        return DXTRACE_ERR(TEXT("SetFormat"), hr);

    SAFE_RELEASE(pDSBPrimary);

    return S_OK;
}

HRESULT CStreamingSound::RecreateBuffers()
{
    DWORD i;
    DSBPOSITIONNOTIFY *aPosNotify;
    LPDIRECTSOUNDNOTIFY pNotify;
    DWORD j;

    this->isPlaying = 0;
    for (i = 0; i < m_dwNumBuffers; i++)
    {
        SAFE_RELEASE(m_apDSBuffer[i]);
    }
    SAFE_DELETE_ARRAY(m_apDSBuffer);

    aPosNotify = NULL;
    pNotify = NULL;
    m_apDSBuffer = new LPDIRECTSOUNDBUFFER[m_dwNumBuffers];
    for (i = 0; i < m_dwNumBuffers; i++)
    {
        if (FAILED(this->manager->m_pDS->CreateSoundBuffer(&this->dsbd, &m_apDSBuffer[i], NULL)))
            return E_FAIL;

        if (FAILED(m_apDSBuffer[i]->QueryInterface(IID_IDirectSoundNotify, (VOID **)&pNotify)))
            return E_FAIL;

        aPosNotify = new DSBPOSITIONNOTIFY[16];
        if (aPosNotify == NULL)
            return E_OUTOFMEMORY;

        for (j = 0; j < 16; j++)
        {
            aPosNotify[j].dwOffset = (m_dwNotifySize * j) + m_dwNotifySize - 1;
            aPosNotify[j].hEventNotify = this->hNotifyEvent;
        }

        if (FAILED(pNotify->SetNotificationPositions(16, aPosNotify)))
        {
            SAFE_RELEASE(pNotify);
            SAFE_DELETE(aPosNotify);
            return E_FAIL;
        }

        SAFE_RELEASE(pNotify);
        SAFE_DELETE(aPosNotify);
    }
    return S_OK;
}
