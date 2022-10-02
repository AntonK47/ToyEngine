#pragma once

#include <vector>

#include "Common.h"
#include "WindowIO.h"

namespace toy::window
{
    struct WindowHandler
    {
#ifdef WIN32
        HWND hwnd;
        HINSTANCE hinstance;
#endif
    };

    struct BackendRendererMeta
    {
#ifdef TOY_ENGINE_VULKAN_BACKEND
        std::vector<const char*> requiredExtensions;
#endif
    };

    enum class Event
    {
        quit
    };

    struct WindowDescriptor
    {
        toy::core::u32 width;
        toy::core::u32 height;
    };

    class Window
    {
    public:
        virtual ~Window() = default;

        virtual void pollEvents() = 0;
        [[nodiscard]] virtual std::vector<Event> getEvents() = 0;
        [[nodiscard]] virtual io::WindowIo getIO() = 0;
        virtual void resize(core::u32 width, core::u32 height) = 0;

        [[nodiscard]] WindowHandler getWindowHandler() const
        {
            return handler_;
        }

        [[nodiscard]] BackendRendererMeta getRendererMeta() const
        {
            return meta_;
        }

        [[nodiscard]] core::u32 width() const { return width_; }
        [[nodiscard]] core::u32 height() const { return height_; }


        void initialize(const WindowDescriptor& descriptor);
        void deinitialize();

    protected:
        virtual void initializeInternal(const WindowDescriptor& descriptor) = 0;
        virtual void deinitializeInternal() = 0;

        WindowHandler handler_{};
        BackendRendererMeta meta_{};
        core::u32 width_;
        core::u32 height_;
    };

    inline void Window::initialize(const WindowDescriptor& descriptor)
    {
        width_ = descriptor.width;
        height_ = descriptor.height;

        initializeInternal(descriptor);
    }

    inline void Window::deinitialize()
    {
        deinitializeInternal();
    }
}
