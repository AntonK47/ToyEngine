#include "SDLWindow.h"

#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_syswm.h>

using namespace toy::window;
using namespace toy::core;

SDLWindow::SDLWindow(const u32 width, const u32 height) : Window(width, height)
{
    {
        auto result = SDL_Init(SDL_INIT_VIDEO);
        if (!result)
        {
            //TODO: LOG ERROR
        }
#ifdef TOY_ENGINE_VULKAN_BACKEND
        window_ = SDL_CreateWindow("Vulkan Window",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            static_cast<int>(width_),
            static_cast<int>(height_),
            SDL_WINDOW_VULKAN);
        if (!result)
        {
            //TODO: LOG ERROR
        }

        auto extensionsCount = u32{};
        result = SDL_Vulkan_GetInstanceExtensions(window_, &extensionsCount, nullptr);
        if (!result)
        {
            //TODO: LOG ERROR
        }
        auto extensions = std::vector<const char*>(extensionsCount);
        result = SDL_Vulkan_GetInstanceExtensions(window_, &extensionsCount, extensions.data());
        if (!result)
        {
            //TODO: LOG ERROR
        }

        meta_.requiredExtensions = extensions;
#endif

#ifdef WIN32
        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        SDL_GetWindowWMInfo(window_, &info);

        handler_.hwnd = info.info.win.window;
        handler_.hinstance = info.info.win.hinstance;

#endif

        currentPolledEvents_ = std::vector<Event>{};
    }
}

void SDLWindow::resize(core::u32 width, core::u32 height)
{
	
}

void SDLWindow::resetPolledEventsAndIo()
{
    currentPolledEvents_.clear();
    windowIo_.keyboardState.reset();
    windowIo_.mouseState.reset();
}

SDLWindow::~SDLWindow()
{
    SDL_DestroyWindow(window_);
    SDL_Quit();
}