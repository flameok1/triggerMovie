#pragma once
#include <windows.h>
#include <mfapi.h>
#include <mfplay.h>
#include <mfreadwrite.h>
#include <shlwapi.h>
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>


/// <summary>
/// ¼·©ñvideoªº player
/// </summary>
class VideoPlayer : public IMFPMediaPlayerCallback
{
public:
    VideoPlayer(HWND hwnd);

    ~VideoPlayer();

    bool Init();

    bool PlayNext(int playType);

    void UpdateVideo();

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
    {
        if (__uuidof(IMFPMediaPlayerCallback) == riid || __uuidof(IUnknown) == riid)
        {
            *ppvObject = static_cast<IMFPMediaPlayerCallback*>(this);
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return InterlockedIncrement(&m_cRef);
    }

    ULONG STDMETHODCALLTYPE Release(void)
    {
        ULONG cRef = InterlockedDecrement(&m_cRef);
        if (cRef == 0)
        {
            delete this;
        }
        return cRef;
    }


    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual void OnMediaPlayerEvent(MFP_EVENT_HEADER* pEventHeader);

    int getNowType();

    void setNextType(int playType);
private:
    HWND hwnd;
    IMFPMediaPlayer* pPlayer;
    std::vector<std::wstring> videoList[2];
    size_t currentIndex;
    long m_cRef = 1;
    int _playType;
    int _nextPlayType;
};