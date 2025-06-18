#pragma once
#include <string>
#include <windows.h>


typedef LRESULT (*WINPROC_CB)(HWND, UINT, WPARAM, LPARAM);

class CWindow
{
public:
    CWindow(const std::wstring& title, int width, int height, WINPROC_CB cb, DWORD dwStyle = WS_OVERLAPPEDWINDOW);
    ~CWindow();
private:
	HINSTANCE _hInst;
    std::wstring _title;
    const wchar_t* szWindowClass = L"WindowClass";
    int _width;
    int _height;
    HWND _hWnd;
    DWORD _dwStyle;
    RECT _windowRect;
    bool _isFullScreen;

    LRESULT(*_WndProc)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


    BOOL InitInstance();

    void destroy();
public:

    void show(int cmdShow = SW_SHOWNORMAL);

    bool isRunning() const;

    /// <summary>
    /// 標準訊息處理
    /// </summary>
    int runMessageLoop();

    HWND getHwnd() const;

    void  ToggleFullscreen();
};