#include "pch.h"
#include "CWindow.h"


CWindow::CWindow(const std::wstring& title, int width, int height, WINPROC_CB cb , DWORD dwStyle)
	: _title(title), _width(width), _height(height), _hWnd(nullptr), _WndProc(cb), _dwStyle(dwStyle)
{
    _isFullScreen = false;
    InitInstance();
}

CWindow::~CWindow() {
    destroy();
}

BOOL CWindow::InitInstance()
{
    _hInst = GetModuleHandle(nullptr);

    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = _WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = _hInst;
    wcex.hIcon = nullptr;
    wcex.hCursor = nullptr;
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = nullptr;

    RegisterClassExW(&wcex);


    _hWnd = CreateWindowW(szWindowClass, _title.c_str(), _dwStyle,
        CW_USEDEFAULT, CW_USEDEFAULT, _width, _height, nullptr, nullptr, _hInst, nullptr);

    if (!_hWnd)
    {
        return FALSE;
    }

    return TRUE;
}


void CWindow::destroy() {
    if (_hWnd) {
        DestroyWindow(_hWnd);
        _hWnd = nullptr;
    }
}

void CWindow::show(int cmdShow) {
    ShowWindow(_hWnd, cmdShow);
    UpdateWindow(_hWnd);

    GetWindowRect(_hWnd, &_windowRect);
}

bool CWindow::isRunning() const {
    MSG msg;
    return (GetMessageW(&msg, nullptr, 0, 0) > 0);
}

int CWindow::runMessageLoop() {
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}

HWND CWindow::getHwnd() const {
    return _hWnd;
}

void  CWindow::ToggleFullscreen() {
    if (!_isFullScreen) {
        // 儲存原本的視窗位置與樣式
        GetWindowRect(_hWnd, &_windowRect);
        _dwStyle = GetWindowLong(_hWnd, GWL_STYLE);

        // 移除邊框與標題列
        SetWindowLong(_hWnd, GWL_STYLE, _dwStyle & ~(WS_OVERLAPPEDWINDOW));

        // 設為全螢幕
        SetWindowPos(_hWnd, HWND_TOP,
            0, 0,
            GetSystemMetrics(SM_CXSCREEN),
            GetSystemMetrics(SM_CYSCREEN),
            SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

        _isFullScreen = !_isFullScreen;
    }
    else {
        // 還原原始視窗樣式與大小
        SetWindowLong(_hWnd, GWL_STYLE, _dwStyle);

        SetWindowPos(_hWnd, NULL,
            _windowRect.left, _windowRect.top,
            _windowRect.right - _windowRect.left,
            _windowRect.bottom - _windowRect.top,
            SWP_NOZORDER | SWP_FRAMECHANGED);

        _isFullScreen = !_isFullScreen;
    }
}