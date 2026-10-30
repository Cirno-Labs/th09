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

HRESULT CSound::RestoreBuffer(LPDIRECTSOUNDBUFFER pDSB, BOOL *pbWasRestored)
{
    HRESULT hr;

    if (pDSB == NULL)
        return CO_E_NOTINITIALIZED;
    if (pbWasRestored)
        *pbWasRestored = FALSE;

    DWORD dwStatus;
    if (FAILED(hr = pDSB->GetStatus(&dwStatus)))
        return DXTRACE_ERR(TEXT("GetStatus"), hr);

    if (dwStatus & DSBSTATUS_BUFFERLOST)
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so
        // the restoring the buffer may fail.
        // If it does, sleep until DirectSound gives us control.
        do
        {
            hr = pDSB->Restore();
            if (hr == DSERR_BUFFERLOST)
                Sleep(10);
        } while (hr = pDSB->Restore());

        if (pbWasRestored != NULL)
            *pbWasRestored = TRUE;

        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

LPDIRECTSOUNDBUFFER CSound::GetFreeBuffer()
{
    BOOL bIsPlaying = FALSE;

    if (m_apDSBuffer == NULL)
        return FALSE;

    for (DWORD i = 0; i < m_dwNumBuffers; i++)
    {
        if (m_apDSBuffer[i])
        {
            DWORD dwStatus = 0;
            m_apDSBuffer[i]->GetStatus(&dwStatus);
            if ((dwStatus & DSBSTATUS_PLAYING) == 0)
                break;
        }
    }

    if (i != m_dwNumBuffers)
        return m_apDSBuffer[i];
    else
        return m_apDSBuffer[rand() % m_dwNumBuffers];
}

LPDIRECTSOUNDBUFFER CSound::GetBuffer(DWORD dwIndex)
{
    if (m_apDSBuffer == NULL)
        return NULL;
    if (dwIndex >= m_dwNumBuffers)
        return NULL;

    return m_apDSBuffer[dwIndex];
}

HRESULT CSound::SetVolume(LONG lVolume)
{
    float scale = g_SoundPlayer.bgmVolume / 100.0f;

    if (g_SoundPlayer.bgmVolume != 0)
    {
        scale = (scale = 1.0f - scale) * scale;
        scale = 1.0f - scale;
        return m_apDSBuffer[0]->SetVolume((LONG)((lVolume + 5000) * scale) - 5000);
    }
    else
    {
        return m_apDSBuffer[0]->SetVolume(DSBVOLUME_MIN);
    }
}

HRESULT CSound::Stop()
{
    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;

    HRESULT hr = 0;

    this->isPlaying = 0;
    this->played = 0;
    for (DWORD i = 0; i < m_dwNumBuffers; i++)
    {
        hr |= m_apDSBuffer[i]->Stop();
        hr |= m_apDSBuffer[i]->SetCurrentPosition(0);
    }
    this->fading = 0;

    return hr;
}

HRESULT CSound::PauseBuffer()
{
    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;

    if (!this->played)
        return S_OK;

    HRESULT hr = 0;

    this->isPlaying = 0;
    hr |= m_apDSBuffer[0]->Stop();

    return hr;
}

HRESULT CSound::ResumeBuffer()
{
    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;
    if (!this->played)
        return S_OK;

    LPDIRECTSOUNDBUFFER pDSB = m_apDSBuffer[0];

    this->isPlaying = 1;
    return pDSB->Play(0, this->dwPlayPriority, this->dwPlayFlags);
}

HRESULT CSound::Reset()
{
    if (m_apDSBuffer == NULL)
        return CO_E_NOTINITIALIZED;

    HRESULT hr = 0;

    for (DWORD i = 0; i < m_dwNumBuffers; i++)
        hr |= m_apDSBuffer[i]->SetCurrentPosition(0);

    return hr;
}

int CStreamingSound::UpdateFade()
{
    LONG lVolume;
    HRESULT hr;

    if (this->fading == 1)
    {
        this->fadeFrames--;
        if (this->fadeFrames <= 0)
        {
            this->fading = 0;
            m_apDSBuffer[0]->Stop();
            return 1;
        }
        lVolume = this->fadeFrames * 5000 / this->fadeTotal - 5000;
        hr = SetVolume(lVolume);
    }
    return 0;
}

int CStreamingSound::UpdateFadeIn()
{
    LONG lVolume;
    HRESULT hr;

    if (this->fading == 2)
    {
        this->fadeFrames--;
        if (this->fadeFrames <= 0)
        {
            this->fading = 0;
            return 1;
        }
        lVolume = 0 - this->fadeFrames * 5000 / this->fadeTotal;
        hr = SetVolume(lVolume);
    }
    return 0;
}

int CStreamingSound::UpdateFadeUp()
{
    LONG lVolume;
    HRESULT hr;

    if (this->fading == 3)
    {
        this->fadeFrames--;
        if (this->fadeFrames <= 0)
        {
            this->fading = 0;
            return 1;
        }
        lVolume = 0 - this->fadeFrames * 1000 / this->fadeTotal;
        hr = SetVolume(lVolume);
    }
    return 0;
}

int CStreamingSound::UpdateFadeDown()
{
    LONG lVolume;
    HRESULT hr;

    if (this->fading == 4)
    {
        this->fadeFrames--;
        if (this->fadeFrames <= 0)
        {
            this->fading = 0;
            return 1;
        }
        lVolume = this->fadeFrames * 1000 / this->fadeTotal - 1000;
        hr = SetVolume(lVolume);
    }
    return 0;
}
