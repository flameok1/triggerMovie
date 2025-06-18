#include "pch.h"
#include "VideoPlayer.h"

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfplay.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "shlwapi.lib")

namespace fs = std::filesystem;

VideoPlayer::VideoPlayer(HWND hwnd)
    : hwnd(hwnd), pPlayer(nullptr), currentIndex(0)
{
    _playType = 0;
    _nextPlayType = 0;
}

VideoPlayer::~VideoPlayer()
{
    if (pPlayer) {
        pPlayer->Shutdown();
        pPlayer->Release();
    }
    MFShutdown();
}


bool VideoPlayer::Init() {
    if (FAILED(MFStartup(MF_VERSION))) return false;

    if (fs::exists("movie") && fs::is_directory("movie"))
    {
        for (const auto& entry : fs::directory_iterator("movie")) {
            if (entry.is_regular_file()) {
                std::wstring ext = entry.path().extension().wstring();
                if (_wcsicmp(ext.c_str(), L".mp4") == 0 || _wcsicmp(ext.c_str(), L".avi") == 0 || _wcsicmp(ext.c_str(), L".wmv") == 0) {
                    videoList[0].push_back(entry.path().wstring());
                }
            }
        }
    }

    if (fs::exists("movie2") && fs::is_directory("movie2"))
    {
        for (const auto& entry : fs::directory_iterator("movie2")) {
            if (entry.is_regular_file()) {
                std::wstring ext = entry.path().extension().wstring();
                if (_wcsicmp(ext.c_str(), L".mp4") == 0 || _wcsicmp(ext.c_str(), L".avi") == 0 || _wcsicmp(ext.c_str(), L".wmv") == 0) {
                    videoList[1].push_back(entry.path().wstring());
                }
            }
        }
    }


    if (videoList[0].empty()) {
        MessageBox(hwnd, L"No video files found in 'movie' folder.", L"Error", MB_OK);
        return false;
    }

    if (videoList[1].empty()) {
        MessageBox(hwnd, L"No video files found in 'movie2' folder.", L"Error", MB_OK);
        return false;
    }

    return PlayNext(0);
}

bool VideoPlayer::PlayNext(int type) {
    if (pPlayer) {
        pPlayer->Shutdown();
        pPlayer->Release();
        pPlayer = nullptr;
    }

    if (_playType != type)
    {
        _playType = type;
        _nextPlayType = _playType;
        if (_playType >= 2 || _playType < 0)
        {
            _playType = 0;
            _nextPlayType = _playType;
        }

        currentIndex = 0;
    }
    else
    {
        currentIndex = (currentIndex + 1) % videoList[_playType].size();
    }

    std::wstring& file = videoList[_playType][currentIndex];

    HRESULT hr = MFPCreateMediaPlayer(
        file.c_str(),   // File path
        TRUE,          // auto play
        0,              // Flags
        this,           // Callback
        hwnd,           // Window handle
        &pPlayer        // Out player
    );

    return SUCCEEDED(hr);
}

void VideoPlayer::OnMediaPlayerEvent(MFP_EVENT_HEADER* pEventHeader)
{
    switch (pEventHeader->eEventType)
    {
    case MFP_EVENT_TYPE_MEDIAITEM_CREATED:
        std::cout << "Event: Media Item Created" << std::endl;
        break;

    case MFP_EVENT_TYPE_PLAY:
        std::cout << "Event: Play Started" << std::endl;
        break;

    case MFP_EVENT_TYPE_PAUSE:
        std::cout << "Event: Play Paused" << std::endl;
        break;

    case MFP_EVENT_TYPE_STOP:
        std::cout << "Event: Play Stopped" << std::endl;
        break;

    case MFP_EVENT_TYPE_ERROR:
        std::cerr << "Event: Error Occurred (HRESULT: " << std::hex << pEventHeader->hrEvent << ")" << std::endl;
        break;

    case MFP_EVENT_TYPE_MEDIAITEM_SET:
        std::cout << "Event: Media Item Set" << std::endl;
        break;

    case MFP_EVENT_TYPE_PLAYBACK_ENDED:
        PlayNext(_nextPlayType);
        break;

    default:
        std::cout << "Event: Unknown Event Type (" << pEventHeader->eEventType << ")" << std::endl;
        break;
    }
}

void VideoPlayer::UpdateVideo()
{
    
}

int VideoPlayer::getNowType()
{
    return _playType;
}

void VideoPlayer::setNextType(int playType)
{
    _nextPlayType = playType;
}

LRESULT CALLBACK VideoPlayer::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static VideoPlayer* player = nullptr;

    switch (message) {
    case WM_CREATE:
        player = new VideoPlayer(hWnd);
        if (!player->Init()) PostQuitMessage(0);
        return 0;

    case WM_DESTROY:
        delete player;
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}