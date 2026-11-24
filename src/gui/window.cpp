/**
 * Window Implementation - Win32 GDI Backend
 *
 * Provides native 2D graphical window rendering and keyboard input on Windows
 * without requiring external dependencies like SDL2.
 */
#include "window.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <windows.h>

static const char* WINDOW_CLASS_NAME = "CINEBREW_WINDOW_CLASS";

struct Win32RenderContext {
    HDC hdcMem;
    HBITMAP hbmpMem;
    HBRUSH hBrush;
    HPEN hPen;
    COLORREF currentColor;
};

static int mapVkToScanCode(WPARAM vk) {
    switch (vk) {
        case VK_LEFT:   return KEY_LEFT;     // 80
        case VK_UP:     return KEY_UP;       // 82
        case VK_RIGHT:  return KEY_RIGHT;    // 79
        case VK_DOWN:   return KEY_DOWN;     // 81
        case VK_SPACE:  return KEY_SPACE;    // 44
        case VK_RETURN: return KEY_RETURN;   // 40
        case VK_ESCAPE: return KEY_ESCAPE;   // 41
        case VK_TAB:    return KEY_TAB;      // 43
        case VK_BACK:   return KEY_BACKSPACE;// 42
        default:
            if (vk >= 'A' && vk <= 'Z') {
                return KEY_A + (vk - 'A');   // 4..29
            }
            if (vk >= '1' && vk <= '9') {
                return KEY_1 + (vk - '1');   // 30..38
            }
            if (vk == '0') return KEY_0;      // 39
            return 0;
    }
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window* self = reinterpret_cast<Window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));

    switch (uMsg) {
        case WM_CLOSE:
        case WM_DESTROY:
            if (self) {
                self->close();
            }
            return 0;

        case WM_KEYDOWN:
        case WM_KEYUP: {
            if (self) {
                int scanCode = mapVkToScanCode(wParam);
                if (scanCode >= 0 && scanCode < 256) {
                    self->setKeyState(scanCode, uMsg == WM_KEYDOWN);
                }
            }
            return 0;
        }
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}
#endif

bool Window::initialize() {
#ifdef _WIN32
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    
    RegisterClassA(&wc);
#endif
    return true;
}

void Window::shutdown() {
#ifdef _WIN32
    UnregisterClassA(WINDOW_CLASS_NAME, GetModuleHandleA(NULL));
#endif
}

Window::Window(int width, int height, const std::string& title)
    : window_(nullptr), renderer_(nullptr), width_(width), height_(height), isOpen_(true) {
    std::memset(keys_, 0, sizeof(keys_));

#ifdef _WIN32
    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);
    int winW = rect.right - rect.left;
    int winH = rect.bottom - rect.top;

    HWND hwnd = CreateWindowExA(
        0, WINDOW_CLASS_NAME, title.c_str(),
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, winW, winH,
        NULL, NULL, GetModuleHandleA(NULL), NULL
    );

    if (hwnd) {
        window_ = hwnd;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        HDC hdcWin = GetDC(hwnd);
        HDC hdcMem = CreateCompatibleDC(hdcWin);
        HBITMAP hbmpMem = CreateCompatibleBitmap(hdcWin, width, height);
        SelectObject(hdcMem, hbmpMem);
        ReleaseDC(hwnd, hdcWin);

        Win32RenderContext* ctx = new Win32RenderContext();
        ctx->hdcMem = hdcMem;
        ctx->hbmpMem = hbmpMem;
        ctx->currentColor = RGB(255, 255, 255);
        ctx->hBrush = CreateSolidBrush(ctx->currentColor);
        ctx->hPen = CreatePen(PS_SOLID, 1, ctx->currentColor);

        SelectObject(ctx->hdcMem, ctx->hBrush);
        SelectObject(ctx->hdcMem, ctx->hPen);

        renderer_ = ctx;

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    } else {
        isOpen_ = false;
    }
#else
    std::cout << "[GUI] Created stub window: " << title << " (" << width << "x" << height << ")" << std::endl;
#endif
}

Window::~Window() {
#ifdef _WIN32
    if (renderer_) {
        Win32RenderContext* ctx = static_cast<Win32RenderContext*>(renderer_);
        if (ctx->hBrush) DeleteObject(ctx->hBrush);
        if (ctx->hPen) DeleteObject(ctx->hPen);
        if (ctx->hbmpMem) DeleteObject(ctx->hbmpMem);
        if (ctx->hdcMem) DeleteDC(ctx->hdcMem);
        delete ctx;
        renderer_ = nullptr;
    }
    if (window_) {
        DestroyWindow(static_cast<HWND>(window_));
        window_ = nullptr;
    }
#endif
}

void Window::close() {
    isOpen_ = false;
}

void Window::clear() {
#ifdef _WIN32
    if (!renderer_) return;
    Win32RenderContext* ctx = static_cast<Win32RenderContext*>(renderer_);
    RECT rect = { 0, 0, width_, height_ };
    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(ctx->hdcMem, &rect, blackBrush);
    DeleteObject(blackBrush);
#endif
}

void Window::present() {
#ifdef _WIN32
    if (!window_ || !renderer_) return;
    HWND hwnd = static_cast<HWND>(window_);
    Win32RenderContext* ctx = static_cast<Win32RenderContext*>(renderer_);
    HDC hdcWin = GetDC(hwnd);
    BitBlt(hdcWin, 0, 0, width_, height_, ctx->hdcMem, 0, 0, SRCCOPY);
    ReleaseDC(hwnd, hdcWin);
#endif
}

void Window::setColor(int r, int g, int b) {
#ifdef _WIN32
    if (!renderer_) return;
    Win32RenderContext* ctx = static_cast<Win32RenderContext*>(renderer_);
    ctx->currentColor = RGB(r, g, b);

    if (ctx->hBrush) DeleteObject(ctx->hBrush);
    if (ctx->hPen) DeleteObject(ctx->hPen);

    ctx->hBrush = CreateSolidBrush(ctx->currentColor);
    ctx->hPen = CreatePen(PS_SOLID, 1, ctx->currentColor);

    SelectObject(ctx->hdcMem, ctx->hBrush);
    SelectObject(ctx->hdcMem, ctx->hPen);
#endif
}

void Window::drawRectangle(int x, int y, int w, int h) {
#ifdef _WIN32
    if (!renderer_) return;
    Win32RenderContext* ctx = static_cast<Win32RenderContext*>(renderer_);
    Rectangle(ctx->hdcMem, x, y, x + w, y + h);
#endif
}

void Window::drawCircle(int x, int y, int radius) {
#ifdef _WIN32
    if (!renderer_) return;
    Win32RenderContext* ctx = static_cast<Win32RenderContext*>(renderer_);
    Ellipse(ctx->hdcMem, x - radius, y - radius, x + radius, y + radius);
#endif
}

void Window::drawLine(int x1, int y1, int x2, int y2) {
#ifdef _WIN32
    if (!renderer_) return;
    Win32RenderContext* ctx = static_cast<Win32RenderContext*>(renderer_);
    MoveToEx(ctx->hdcMem, x1, y1, NULL);
    LineTo(ctx->hdcMem, x2, y2);
#endif
}

bool Window::processEvents() {
#ifdef _WIN32
    if (!window_) return false;
    HWND hwnd = static_cast<HWND>(window_);
    MSG msg;
    while (PeekMessageA(&msg, hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
#endif
    return isOpen_;
}

bool Window::isKeyPressed(int keyCode) const {
    if (keyCode < 0 || keyCode >= 256) return false;
    return keys_[keyCode];
}
