#include "Win32Window.h"

#include <oleidl.h>
#include <format>
#include <iostream>

using namespace toy::window;
using namespace toy::core;

namespace
{
    

    LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        Win32Window* self = nullptr;
        if (msg == WM_NCCREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            self = (Win32Window*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)self);

        }
        else
        {
            self = (Win32Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }
        if (self)
        {
            self->handleMessage(msg, wParam, lParam);
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

void Win32Window::initializeInternal(const WindowDescriptor& descriptor)
{
    OleInitialize(NULL);

    const char CLASS_NAME[] = "Sample Window Class";

    WNDCLASS wc = { };
    HINSTANCE hInstance = GetModuleHandle(NULL);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    DWORD styleEx = 0;


    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = (LONG)width_;
    rect.bottom = (LONG)height_;
    AdjustWindowRectEx(&rect, style, FALSE, styleEx);

    auto x = rect.left;
    auto y = rect.top;
    auto w = (rect.right - rect.left);
    auto h = (rect.bottom - rect.top);

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        "Sample Window Class",                     // Window class
        "Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, w, h,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        this        // Additional application data
    );
    ShowWindow(hwnd, SW_SHOW);
    {
        
        meta_.requiredExtensions = {  "VK_KHR_surface", "VK_KHR_win32_surface" };


        handler_.hwnd = hwnd;
        handler_.hinstance = hInstance;
        currentPolledEvents_ = std::vector<Event>{};//TODO: smallvector

        windowIo_.keyboardState.reset();
		windowIo_.mouseState.reset();
        windowIo_.textState.reset();
    }

    
}


void Win32Window::resetPolledEventsAndIo()
{
    if (windowIo_.keyboardState.b == toy::io::ButtonState::pressed)
    {
        
    }
    if (windowIo_.keyboardState.n == toy::io::ButtonState::pressed)
    {
        
    }
    currentPolledEvents_.clear();
    windowIo_.textState.reset();
    /*windowIo_.keyboardState.reset();
    windowIo_.mouseState.reset();*/
}

void Win32Window::setWindowTitleInternal(const std::string& title)
{
    
}

toy::io::WindowIo Win32Window::getIoInternal()
{
	return windowIo_;
}

std::vector<Event> Win32Window::getEventsInternal()//TODO: smallvector
{
	return currentPolledEvents_;
}

void Win32Window::deinitializeInternal()
{
    
}

void Win32Window::resizeInternal(core::u32 width, core::u32 height)
{
    
}
