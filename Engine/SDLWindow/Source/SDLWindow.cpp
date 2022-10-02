#include "SDLWindow.h"
#define SDL_MAIN_HANDLED


#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_syswm.h>
#include <Logger.h>

using namespace toy::window;
using namespace toy::core;

void SDLWindow::initializeInternal(const WindowDescriptor& descriptor)
{
    {
        auto result = SDL_Init(SDL_INIT_VIDEO);
        if (result)
        {
            LOG(FATAL) << SDL_GetError();
        }
#ifdef TOY_ENGINE_VULKAN_BACKEND
        window_ = SDL_CreateWindow("Vulkan Window",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            static_cast<int>(width_),
            static_cast<int>(height_),
            SDL_WINDOW_VULKAN);

        auto extensionsCount = u32{};
        result = SDL_Vulkan_GetInstanceExtensions(window_, &extensionsCount, nullptr);
        if (result == SDL_FALSE)
        {
            LOG(FATAL) << SDL_GetError();
        }
        auto extensions = std::vector<const char*>(extensionsCount);
        result = SDL_Vulkan_GetInstanceExtensions(window_, &extensionsCount, extensions.data());
        if (result == SDL_FALSE)
        {
            LOG(FATAL) << SDL_GetError();
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

void SDLWindow::resizeInternal(u32 width, u32 height)
{
	
}

void SDLWindow::resetPolledEventsAndIo()
{
    currentPolledEvents_.clear();
    windowIo_.keyboardState.reset();
    windowIo_.mouseState.reset();
}

toy::io::WindowIo SDLWindow::getIoInternal()
{
	return windowIo_;
}

std::vector<Event> SDLWindow::getEventsInternal()
{
	return currentPolledEvents_;
}

void SDLWindow::deinitializeInternal()
{
    SDL_DestroyWindow(window_);
    SDL_Quit();
}
